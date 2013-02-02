/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
// Declare the public types for oMath. This header can be included separately 
// from oMath to keep public header contents simple. The basic types are 
// intended to be the same as in HLSL Shader Model 5.0.
#pragma once
#ifndef oMathTypes_h
#define oMathTypes_h

#include <oBasis/oColor.h>
#include <oBasis/oHLSLMath.h>
#include <oBasis/oOperators.h>
#include <oBasis/oTypes.h>

typedef TVEC2<char> char2; typedef TVEC2<uchar> uchar2;
typedef TVEC3<char> char3; typedef TVEC3<uchar> uchar3;
typedef TVEC4<char> char4; typedef TVEC4<uchar> uchar4;

// _____________________________________________________________________________
// HLSL-like types not in the HLSL specification

typedef TVEC2<short> short2; typedef TVEC2<ushort> ushort2;
typedef TVEC3<short> short3; typedef TVEC3<ushort> ushort3;
typedef TVEC4<short> short4; typedef TVEC4<ushort> ushort4;

typedef TVEC2<llong> llong2; typedef TVEC2<ullong> ullong2;
typedef TVEC3<llong> llong3; typedef TVEC3<ullong> ullong3;
typedef TVEC4<llong> llong4; typedef TVEC4<ullong> ullong4;

// _____________________________________________________________________________
// Additional types

struct oRGBf
{
	// Handle automatic expansion of oColor to float3s. Using this type allows for 
	// a single header to be defined for usage in both C++ and HLSL. NOTE: Alpha
	// is quietly dropped/ignored when using this - see oColorDecomposeRGB for 
	// more information.

	oRGBf() : Color(0.0f, 0.0f, 0.0f) {}
	oRGBf(float _R, float _G, float _B) : Color(_R, _G, _B) {}
	oRGBf(const float3& _Color) : Color(_Color) {}
	oRGBf(const oColor& _Color) : Color(oColorDecomposeRGB<float3>(_Color)) {}
	oRGBf(const oRGBf& _RGB) : Color(_RGB.Color) {}

	inline operator float3&() { return Color; }
	inline operator const float3&() const { return Color; }
	inline operator oColor() const { return oColorCompose(Color.x, Color.y, Color.z, 1.0f); }
	inline const oRGBf& operator=(int _Color) { Color = (float)_Color; return *this; }
	inline const oRGBf& operator=(const oRGBf& _Color) { Color = _Color.Color; return *this; }
	inline const oRGBf& operator=(const float3& _Color) { Color = _Color; return *this; }
	inline const oRGBf& operator=(const oColor& _Color) { Color = oColorDecomposeRGB<float3>(_Color); return *this; }

	const oRGBf& operator+=(const oRGBf& _That) { Color = saturate(Color + (const float3&)_That); return *this; }
	const oRGBf& operator-=(const oRGBf& _That) { Color = saturate(Color - (const float3&)_That); return *this; }
	const oRGBf& operator*=(const oRGBf& _That) { Color = saturate(Color * (const float3&)_That); return *this; }
	const oRGBf& operator/=(const oRGBf& _That) { Color = saturate(Color / (const float3&)_That); return *this; }

	oOPERATORS_DERIVED(oRGBf, +) oOPERATORS_DERIVED(oRGBf, -) oOPERATORS_DERIVED(oRGBf, *) oOPERATORS_DERIVED(oRGBf, /)

protected:
	float3 Color;
};

typedef TQUAT<float> quatf; typedef TQUAT<double> quatd;
typedef TFRUSTUM<float> oFrustumf; //typedef TFRUSTUM<double> oFrustumd; // @oooii-tony: Need an oIntersects for double
typedef TSPHERE<float> oSpheref; typedef TSPHERE<double> oSphered;
typedef TPLANE<float> oPlanef; typedef TPLANE<double> oPlaned;
typedef TAABOX<float, TVEC3<float>> oAABoxf; typedef TAABOX<double, TVEC3<double>> oAABoxd;
typedef TAABOX<int, TVEC2<int>> oRECT;
typedef TAABOX<float, TVEC2<float>> oRECTF;

// _____________________________________________________________________________
// type traits

// the static value is true if the specified type is a linear algebra type
// this is modeled after std::is_arithmetic for the linear algrebra types found
// in shader/compute languages as well as appearing commonly in Ouroboros C++.
template<typename T> struct is_linear_algebra
{
	static const bool value = 
		std::is_same<char2,std::remove_cv<T>::type>::value ||
		std::is_same<char3,std::remove_cv<T>::type>::value ||
		std::is_same<char4,std::remove_cv<T>::type>::value ||
		std::is_same<uchar2,std::remove_cv<T>::type>::value ||
		std::is_same<uchar3,std::remove_cv<T>::type>::value ||
		std::is_same<uchar4,std::remove_cv<T>::type>::value ||
		std::is_same<short2,std::remove_cv<T>::type>::value ||
		std::is_same<short3,std::remove_cv<T>::type>::value ||
		std::is_same<short4,std::remove_cv<T>::type>::value ||
		std::is_same<ushort2,std::remove_cv<T>::type>::value ||
		std::is_same<ushort3,std::remove_cv<T>::type>::value ||
		std::is_same<ushort4,std::remove_cv<T>::type>::value ||
		std::is_same<int2,std::remove_cv<T>::type>::value ||
		std::is_same<int3,std::remove_cv<T>::type>::value ||
		std::is_same<int4,std::remove_cv<T>::type>::value ||
		std::is_same<uint2,std::remove_cv<T>::type>::value ||
		std::is_same<uint3,std::remove_cv<T>::type>::value ||
		std::is_same<uint4,std::remove_cv<T>::type>::value ||
		std::is_same<llong2,std::remove_cv<T>::type>::value ||
		std::is_same<llong3,std::remove_cv<T>::type>::value ||
		std::is_same<llong4,std::remove_cv<T>::type>::value ||
		std::is_same<ullong2,std::remove_cv<T>::type>::value ||
		std::is_same<ullong3,std::remove_cv<T>::type>::value ||
		std::is_same<ullong4,std::remove_cv<T>::type>::value ||
		std::is_same<float2,std::remove_cv<T>::type>::value ||
		std::is_same<float3,std::remove_cv<T>::type>::value ||
		std::is_same<float4,std::remove_cv<T>::type>::value ||
		std::is_same<float4x4,std::remove_cv<T>::type>::value;
};

#endif