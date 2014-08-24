/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
// This cpp contains implemenations of to_string and from_string for intrinsic
// types as well as ouro types.

#include <oHLSL/oHLSLMath.h>
#include <oBase/stringize.h>

namespace ouro {

bool from_string(float2* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 2, _StrSource);
}

bool from_string(float3* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 3, _StrSource);
}

bool from_string(float4* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 4, _StrSource);
}

bool from_string(float4x4* _pValue, const char* _StrSource)
{
	// Read in-order, then transpose
	bool result = from_string_float_array((float*)_pValue, 16, _StrSource);
	if (result)
		transpose(*_pValue);
	return result;
}

bool from_string(double4x4* _pValue, const char* _StrSource)
{
	// Read in-order, then transpose
	bool result = from_string_double_array((double*)_pValue, 16, _StrSource);
	if (result)
		transpose(*_pValue);
	return result;
}

#define CHK_MV() do \
	{	if (!_pValue || !_StrSource) return false; \
		_StrSource += strcspn(_StrSource, oDIGIT_SIGNED); \
		if (!*_StrSource) return false; \
	} while (false)

#define CHK_MV_U() do \
	{	if (!_pValue || !_StrSource) return false; \
		_StrSource += strcspn(_StrSource, oDIGIT_UNSIGNED); \
		if (!*_StrSource) return false; \
	} while (false)

bool from_string(int2* _pValue, const char* _StrSource) { CHK_MV(); return 2 == sscanf_s(_StrSource, "%d %d", &_pValue->x, &_pValue->y); }
bool from_string(int3* _pValue, const char* _StrSource) { CHK_MV(); return 3 == sscanf_s(_StrSource, "%d %d %d", &_pValue->x, &_pValue->y, &_pValue->z); }
bool from_string(int4* _pValue, const char* _StrSource) { CHK_MV(); return 4 == sscanf_s(_StrSource, "%d %d %d %d", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w); }
bool from_string(uint2* _pValue, const char* _StrSource) { CHK_MV_U(); return 2 == sscanf_s(_StrSource, "%u %u", &_pValue->x, &_pValue->y); }
bool from_string(uint3* _pValue, const char* _StrSource) { CHK_MV_U(); return 3 == sscanf_s(_StrSource, "%u %u %u", &_pValue->x, &_pValue->y, &_pValue->z); }
bool from_string(uint4* _pValue, const char* _StrSource) { CHK_MV(); return 4 == sscanf_s(_StrSource, "%u %u %u %u", &_pValue->x, &_pValue->y, &_pValue->z, &_pValue->w); }

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const float2& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f", value.x, value.y) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const float3& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f", value.x, value.y, value.z) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const float4& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const double2& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f", value.x, value.y) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const double3& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f", value.x, value.y, value.z) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const double4& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const int2& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%d %d", value.x, value.y) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const int3& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%d %d %d", value.x, value.y, value.z) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const int4& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%d %d %d %d", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const uint2& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u %u", value.x, value.y) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const uint3& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u %u %u", value.x, value.y, value.z) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const uint4& value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%u %u %u %u", value.x, value.y, value.z, value.w) ? _StrDestination : nullptr; }

template<typename T> char* to_stringT(char* _StrDestination, size_t _SizeofStrDestination, const TMAT4<T>& value)
{
	return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f"
		, value.Column0.x, value.Column1.x, value.Column2.x, value.Column3.x
		, value.Column0.y, value.Column1.y, value.Column2.y, value.Column3.y
		, value.Column0.z, value.Column1.z, value.Column2.z, value.Column3.z
		, value.Column0.w, value.Column1.w, value.Column2.w, value.Column3.w) ? _StrDestination : nullptr;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const float4x4& value) { return to_stringT(_StrDestination, _SizeofStrDestination, value); }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const double4x4& value) { return to_stringT(_StrDestination, _SizeofStrDestination, value); }

}
