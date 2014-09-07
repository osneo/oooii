// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_json_h
#define oString_json_h

// Parses a string as a JSON document by replacing certain delimiters inline 
// with null terminators and caching indices into the buffers where values
// begin for very fast access to contents.

#include <oString/text_document.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ouro {

enum class json_node_type { object, array, value };
enum class json_value_type { string, number, object, array, true_, false_, null };

class json
{
public:
	typedef uint32_t index_type;

	typedef struct node__ {}* node;

	json() : size_(0) {}
	json(const char* uri, char* data, deallocate_fn deallocate, size_t est_num_nodes = 100)
		: buffer(uri, data, deallocate)
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		nodes.reserve(est_num_nodes);
		index_buffer();
		size_ += nodes.capacity() * sizeof(index_type) * 7;
	}

	json(const char* uri, const char* data, const allocator& alloc = default_allocator, size_t est_num_nodes = 100)
		: buffer(uri, data, alloc, "json doc")
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		nodes.reserve(est_num_nodes);
		index_buffer();
		size_ += nodes.capacity() * sizeof(index_type) * 7;
	}

	json(json&& that) { operator=(std::move(that)); }
	json& operator=(json&& that)
	{
		if (this != &that)
		{
			buffer = std::move(that.buffer);
			nodes = std::move(that.nodes);
			size_ = std::move(that.size_);
		}
		return *this;
	}

	inline operator bool() const { return (bool)buffer; }
	inline const char* name() const { return buffer.uri; }
	inline size_t size() const { return size_; }

	// Node API
	inline node root() const { return node(1); } 
	inline node first_child(node parent_node) const { return node(Node(parent_node).down); }
	inline node next_sibling(node prior_sibling) const { return node(Node(prior_sibling).next); }
	inline const char* node_name(node n) const { return buffer.data + Node(n).name; }
	inline const char* node_value(node n) const { return buffer.data + Node(n).value; }
	json_node_type node_type(node n) const { return Node(n).type; }
	json_value_type value_type(node n) const { return Node(n).value_type; }

	// Convenience functions that use the above API
	inline node first_child(node parent_node, const char* name) const
	{
		node n = first_child(parent_node);
		while (n && _stricmp(node_name(n), name))
			n = next_sibling(n);
		return n;
	}

	inline node next_sibling(node prior_sibling, const char* name) const
	{
		node n = next_sibling(prior_sibling);
		while (n && _stricmp(node_name(n), name))
			n = next_sibling(n);
		return n;
	}

private:
	detail::text_buffer buffer;

	struct node_t
	{
		node_t() : next(0), down(0), attr(0), name(0), value(0), type(json_node_type::object), value_type(json_value_type::object) {}
		index_type next, down, attr, name, value;
		json_node_type type;
		json_value_type value_type;
	};

	typedef std::vector<node_t> nodes_t;

	nodes_t nodes;
	size_t size_;

	inline const node_t& Node(node n) const { return nodes[(size_t)n]; }

	inline node_t& Node(node n) { return nodes[(size_t)n]; }

	// Parsing functions
	inline void index_buffer();
	inline node make_next_node(char*& json_buffer, node parent, node previous, bool is_array, int& open_tag_count, int& close_tag_count);
};

	namespace detail
	{
		inline bool skip_string(char*& json_buffer)
		{
			// json_buffer should point to the first character after the initial quote and 
			// will then skip to the terminating quote.
			do 
			{
				json_buffer += strcspn(json_buffer, "\\\"");
				if (*json_buffer == '\\')
				{
					size_t numBackslashes = strspn(json_buffer, "\\");
					json_buffer += numBackslashes;
					if (*json_buffer == '\"' && (numBackslashes & 1) != 0)
						++json_buffer;
				}
			}
			while (*json_buffer != '\"' && *json_buffer != 0);
			return *json_buffer == '\"';
		}
	} // namespace detail


void json::index_buffer()
{
	nodes.push_back(node_t()); // use up slot 0 so it can be used as a null handle
	nodes.push_back(node_t()); // add root node

	char* start = buffer.data + strcspn(buffer.data, "{[");
	bool isArray = *start++ == '[';
	*buffer.data = 0; // make the first char nul so 0 offsets are the empty string

	int OpenTagCount = 0, CloseTagCount = 0; // these should be equal by the end
	make_next_node(start, root(), 0, isArray, OpenTagCount, CloseTagCount); // start recursing
	if (OpenTagCount != CloseTagCount) throw text_document_error(text_document_errc::unclosed_scope); // if not equal, something went wrong
}

json::node json::make_next_node(char*& json_buffer, node parent, node previous, bool is_array, int& open_tag_count, int& close_tag_count)
{
	open_tag_count++;

	index_type newNode = static_cast<index_type>(nodes.size());
	if (parent) nodes[(size_t)parent].down = newNode;

	node hPreviousNode = previous;
	bool hasMoreSiblings = false;
	do 
	{
		newNode = static_cast<index_type>(nodes.size());
		if (hPreviousNode) nodes[(size_t)hPreviousNode].next = newNode;

		node_t n;
		n.type = json_node_type::value;

		if (is_array)
		{
			n.name = 0;

			// Skip whitespace
			json_buffer += strspn(json_buffer, " \t\v\r\n");
		}
		else
		{
			// Set name without the quotes, so zero terminate on the end quote
			json_buffer += strcspn(json_buffer, "\"");
			n.name = static_cast<index_type>(std::distance(buffer.data, ++json_buffer));
			if (!detail::skip_string(json_buffer))
				return 0;
			*json_buffer++ = 0;

			// Skip to value begin
			json_buffer += strspn(json_buffer, " \t\v\r\n:");
		}

		// Set value
		n.value = static_cast<index_type>(std::distance(buffer.data, json_buffer));

		// Push new node
		nodes.push_back(n);

		node parent = node(newNode);
		node prev = 0;

		// Determine node type and process it
		if (*json_buffer == '{')
		{
			nodes[newNode].type = json_node_type::object;
			nodes[newNode].value = 0;
			nodes[newNode].value_type = json_value_type::object;

			// CreateNextNode sets down and next for newNode
			hPreviousNode = make_next_node(++json_buffer, parent, prev, false, open_tag_count, close_tag_count);
			if (!hPreviousNode) return 0;
		}
		else if (*json_buffer == '[')
		{
			nodes[newNode].type = json_node_type::array;
			nodes[newNode].value = 0;
			nodes[newNode].value_type = json_value_type::array;

			// CreateNextNode sets down and next for newNode
			hPreviousNode = make_next_node(++json_buffer, parent, prev, true, open_tag_count, close_tag_count);
			if (!hPreviousNode) return 0;
		}
		else if (*json_buffer == '\"')
		{
			nodes[newNode].value_type = json_value_type::string;
			detail::skip_string(++json_buffer);
			json_buffer++;
		}
		else if (*json_buffer == 't')
		{
			nodes[newNode].value_type = json_value_type::true_;
			if (0 != memcmp(json_buffer, "true", 4)) return 0;
			json_buffer += 4;
		}
		else if (*json_buffer == 'f')
		{
			nodes[newNode].value_type = json_value_type::false_;
			if (0 != memcmp(json_buffer, "false", 5)) return 0;
			json_buffer += 5;

		}
		else if (*json_buffer == 'n')
		{
			nodes[newNode].value_type = json_value_type::null;
			if (0 != memcmp(json_buffer, "null", 4)) return 0;
			json_buffer += 4;
		}
		else if (*json_buffer != 0)
		{
			nodes[newNode].value_type = json_value_type::number;
			json_buffer += strspn(json_buffer, "0123456789eE+-.");
		}
		else
		{
			return 0;
		}

		char* Marker = json_buffer;

		// Skip whitespace to either the next name/value pair
		// or end of Object/Array marker
		if (is_array)
			json_buffer += strcspn(json_buffer, ",]");
		else
			json_buffer += strcspn(json_buffer, ",}");

		hasMoreSiblings = (*json_buffer == ',');

		// Clear from the marker to current pos to make sure the value above
		// gets zero terminated. We have to do that here because there is not
		// guaranteed room for a zero terminator until we've parsed until here.
		memset(Marker, 0, std::distance(Marker, ++json_buffer));

		hPreviousNode = node(newNode);

	} while (hasMoreSiblings);

	close_tag_count++;

	return (*json_buffer != 0) ? node(newNode) : 0;
}

}

#endif
