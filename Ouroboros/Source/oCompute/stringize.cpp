/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
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

#include <oBase/aabox.h>
#include <oCompute/oFrustum.h>
#include <oCompute/oPlane.h>
#include <oCompute/oQuaternion.h>
#include <oCompute/oSphere.h>
#include <oCompute/rgb.h>
#include <oBase/stringize.h>

namespace ouro {

bool from_string(quatf* _pValue, const char* _StrSource)
{
	return from_string_float_array((float*)_pValue, 4, _StrSource);
}

bool from_string(quatd* _pValue, const char* _StrSource)
{
	return from_string_double_array((double*)_pValue, 4, _StrSource);
}

bool from_string(oPlanef* _pValue, const char* _StrSource)
{
	float4* pTmp = (float4*)_pValue;
	return from_string(pTmp, _StrSource);
}

bool from_string(oSpheref* _pValue, const char* _StrSource)
{
	float4* pTmp = (float4*)_pValue;
	return from_string(pTmp, _StrSource);
}

bool from_string(rgbf* _pValue, const char* _StrSource)
{
	color c;
	// Valid forms are: 0xAABBGGRR, R G B [0,1], and an ouro::color
	if (*_StrSource == '0' && tolower(*_StrSource) == 'x')
	{
		unsigned int i;
		if (from_string(&i, _StrSource))
			*_pValue = *(color*)&i;
	}
	else if (from_string(&c, _StrSource))
		*_pValue = c;
	else if (!from_string((float3*)_pValue, _StrSource))
		return false;
	return true;
}

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatf& _Value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const quatd& _Value) { return -1 != snprintf(_StrDestination, _SizeofStrDestination, "%f %f %f %f", _Value.x, _Value.y, _Value.z, _Value.w) ? _StrDestination : nullptr; }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oPlanef& _Value) { return to_string(_StrDestination, _SizeofStrDestination, (const float4&)_Value); }
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const oSpheref& _Value) { return to_string(_StrDestination, _SizeofStrDestination, (const float4&)_Value); }

char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const rgbf& _Value) 
{
	if (!to_string(_StrDestination, _SizeofStrDestination, (color)_Value))
		if (!to_string(_StrDestination, _SizeofStrDestination, (const float3&)_Value))
			return nullptr;

	return _StrDestination;
}

} // namespace ouro
