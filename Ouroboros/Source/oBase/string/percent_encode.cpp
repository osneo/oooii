// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <oCompiler.h>
#include <oBase/throw.h>

namespace ouro {

char* percent_encode(char* oRESTRICT dst, size_t dst_size, const char* oRESTRICT src, const char* oRESTRICT reserved_chars)
{
	*dst = 0;
	char* d = dst;
	char* end = d + dst_size - 1;
	*(end-1) = 0; // don't allow read outside the buffer even in failure cases

	const char* s = src;
	while (*s)
	{
		if ((*s & 0x80) != 0)
		{
			// TODO: Support UTF-8 
			oTHROW(function_not_supported, "UTF-8 not yet supported");
		}

		if (strchr(reserved_chars, *s))
		{
			if ((d+3) > end)
				oTHROW0(no_buffer_space);
			*d++ = '%';
			snprintf(d, std::distance(d, end), "%02x", *s++); // use lower-case escaping http://www.textuality.com/tag/uri-comp-2.html
			d += 2;
		}

		else if (d >= end)
			oTHROW0(no_buffer_space);
		else
			*d++ = *s++;
	}

	*d = 0;
	return dst;
}

}
