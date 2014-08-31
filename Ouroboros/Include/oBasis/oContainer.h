// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Implementations of common predicates and hashes used in std containers.
#pragma once
#ifndef oContainer_h
#define oContainer_h

#include <xstddef> // for unary_function, binary_function, etc.
#include <oBase/murmur3.h>
#include <oBasis/oInterface.h>

// _____________________________________________________________________________
// Predicates. Structs with an 'I' on the end are case-insensitive

template<class T> struct oStdEquals : public std::unary_function<T, bool>
{	// provide the default operator==
	oStdEquals(const T& _t) : t_(t) {}
	bool operator()(const T& _t) const { return _t == t; }
private:
	oStdEquals() {}
	T t;
};

template<> struct oStdEquals<const char*>
{	// compare strings rather than test pointer equality
	oStdEquals(const char* _String) : String(_String) {}
	bool operator()(const char* _String) { return !strcmp(_String, String); }
private:
	oStdEquals() {}
	const char* String;
};

template<> struct oStdEquals<const std::string>
{
	oStdEquals(const char* _String) : String(_String) {}
	oStdEquals(const std::string& _String) : String(_String.c_str()) {}
	bool operator()(const std::string& _String) { return !_String.compare(String); }
private:
	oStdEquals() {}
	const char* String;
};

template<class T> struct oStdEqualsI : public std::unary_function<T, bool>
{
	oStdEqualsI(const T& _String) : String(_String) {}
	bool operator()(const T& _String) const { return String == _String; }
private:
	oStdEqualsI() {}
	T String;
};

template<> struct oStdEqualsI<const char*>
{
	oStdEqualsI(const char* _String) : String(_String) {}
	bool operator()(const char* _String) { return !_stricmp(String, _String); }
private:
	oStdEqualsI() {}
	const char* String;
};

template<> struct oStdEqualsI<const std::string>
{
	oStdEqualsI(const char* _String) : String(_String) {}
	oStdEqualsI(const std::string& _String) : String(_String.c_str()) {}
	bool operator()(const std::string& _String) { return !_stricmp(_String.c_str(), String); }
private:
	oStdEqualsI() {}
	const char* String;
};

template<typename T> struct oStdLess : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLess<const char*> { int operator()(const char* x, const char* y) const { return strcmp(x, y) < 0; } };
template<typename T> struct oStdLessI : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x < y; } };
template<> struct oStdLessI<const char*> { bool operator()(const char* x, const char* y) const { return _stricmp(x, y) < 0; } };

template<typename T> struct oStdEqualTo : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualTo<const char*> { int operator()(const char* x, const char* y) const { return !strcmp(x, y); } };
template<typename T> struct oStdEqualToI : public std::binary_function<T, T, bool> { bool operator()(const T& x, const T& y) const { return x == y; } };
template<> struct oStdEqualToI<const char*> { bool operator()(const char* x, const char* y) const { return !_stricmp(x, y); } };

// std::hash function that returns exactly the same value as passed in. This is 
// useful where some other object caches hash values that then can be used 
// directly in an unordered_set.
template<typename T> struct oNoopHash : public std::unary_function<T, size_t> { size_t operator()(const T& v) const { return static_cast<size_t>(v); } };
template<typename T> struct oStdHash : std::hash<T> {}; // by default oStdHash is just std hash
template<typename T> struct oStdHashI {}; // by default oStdHashI doesn't work with unordered containers, must specialize

template<typename CHAR_T, size_t CAPACITY>
struct oStdHash<ouro::fixed_string<CHAR_T, CAPACITY>> : public std::unary_function<ouro::fixed_string<CHAR_T, CAPACITY>, size_t>
{
	size_t operator()( const ouro::fixed_string<CHAR_T, CAPACITY>& _Key) const
	{
		uint128 Hash = ouro::murmur3(_Key.c_str(), static_cast<unsigned int>(_Key.length()));
		return *(size_t*)&Hash;
	}
};

template<typename CHAR_T, size_t CAPACITY>
struct oStdHashI<ouro::fixed_string<CHAR_T, CAPACITY>> : public std::unary_function<ouro::fixed_string<CHAR_T, CAPACITY>, size_t>
{
	size_t operator()( const ouro::fixed_string<CHAR_T, CAPACITY>& _Key) const
	{
		ouro::fixed_string<CHAR_T, CAPACITY> lower = _Key;
		ouro::tolower(lower);
		uint128 Hash = ouro::murmur3(lower.c_str(), oUInt(lower.length()));
		return *(size_t*)&Hash;
	}
};

namespace std {
	template <typename T>
	struct hash<TVEC2<T>>
	{
		size_t operator()(const TVEC2<T>& _V) const
		{
			uint128 Hash = ouro::murmur3(&_V, sizeof(_V));
			return *(size_t*)&Hash;
		}
	};

	template <typename T>
	struct hash<TVEC3<T>>
	{
		size_t operator()(const TVEC3<T>& _V) const
		{
			uint128 Hash = ouro::murmur3(&_V, sizeof(_V));
			return *(size_t*)&Hash;
		}
	};

	template <typename T>
	struct hash<TVEC4<T>>
	{
		size_t operator()(const TVEC4<T>& _V) const
		{
			uint128 Hash = ouro::murmur3(&_V, sizeof(_V));
			return *(size_t*)&Hash;
		}
	};

	template<>
	struct hash<oGUID>
	{
		size_t operator()(const oGUID& _GUID) const
		{
			uint128 Hash = ouro::murmur3(&_GUID, sizeof(oGUID));
			return *(size_t*)&Hash;
		}
	};
}

#endif
