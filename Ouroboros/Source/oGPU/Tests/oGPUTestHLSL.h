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
#ifndef oHLSL
	#pragma once
#endif
#ifndef oGPUHLSL_h
#define oGPUHLSL_h

#include <oGfx/oGfxHLSL.h>

struct oGPUTestConstants
{
#ifndef oHLSL

	enum CONSTRUCTION { Identity, };

	oGPUTestConstants() {}
	oGPUTestConstants(CONSTRUCTION _Type) { SetIdentity(); }
	oGPUTestConstants(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, const oRGBAf& _Color) { Set(_World, _View, _Projection, _Color); }

	inline void Set(const float4x4& _World, const float4x4& _View, const float4x4& _Projection, const oRGBAf& _Color)
	{
		World = _World;
		WorldViewProjection = _World * _View * _Projection;
		Color = _Color;
	}

	inline void SetIdentity()
	{
		World = oIDENTITY4x4;
		WorldViewProjection = oIDENTITY4x4;
		Color = ouro::White;
	}

#endif

	float4x4 World;
	float4x4 WorldViewProjection;
	oRGBAf Color;
};

struct oGPU_TEST_INSTANCE
{
	float3 Translation;
	quatf Rotation;
	float PadA;
};

#ifdef oHLSL
cbuffer cbuffer_oGPUTestConstants : register(b0) { oGPUTestConstants GPUTestConstants; }
cbuffer cbuffer_oGPUTestInstances : register(b1) { oGPU_TEST_INSTANCE GPUTestInstances[2]; }

// LS: Local Space
// WS: World Space
// SS: Screen Space
// VS: View Space

struct VSIN
{
	float3 LSPosition : POSITION;
	float3 LSNormal : NORMAL;
	float2 Texcoord : TEXCOORD;
};

struct VSOUT
{
	float4 SSPosition : SV_Position;
	float3 WSPosition : POSITION;
	float3 LSPosition : POSITION1;
	float3 WSNormal : NORMAL;
	float3 Texcoord : TEXCOORD;
	float4 Color : COLOR;
};

struct PSOUT
{
	float4 Color;
};

float4 oGPUTestLStoSS(float3 _LSPosition)
{
	return oMul(GPUTestConstants.WorldViewProjection, float4(_LSPosition, 1));
}

float3 oGPUTestLStoWS(float3 _LSPosition)
{
	return oMul(GPUTestConstants.World, float4(_LSPosition, 1)).xyz;
}

float3 oGPUTestRotateLStoWS(float3 _LSVector)
{
	return oMul(oGetUpper3x3(GPUTestConstants.World), _LSVector);
}

VSOUT CommonVS(VSIN In)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = oGPUTestLStoSS(In.LSPosition);
	Out.WSPosition = oGPUTestLStoWS(In.LSPosition);
	Out.LSPosition = In.LSPosition;
	Out.WSNormal = oGPUTestRotateLStoWS(In.LSNormal);
	Out.Texcoord = float3(In.Texcoord, 0);
	Out.Color = GPUTestConstants.Color;
	return Out;
}

VSOUT CommonVS(float3 _LSPosition, float3 _Texcoord)
{
	VSOUT Out = (VSOUT)0;
	Out.SSPosition = oGPUTestLStoSS(_LSPosition);
	Out.WSPosition = oGPUTestLStoWS(_LSPosition);
	Out.LSPosition = _LSPosition;
	Out.Color = GPUTestConstants.Color;
	Out.Texcoord = _Texcoord;
	return Out;
}

VSOUT CommonVS(float3 _LSPosition, uint _InstanceIndex)
{
	VSOUT Out = (VSOUT)0;
	oGPU_TEST_INSTANCE Inst = GPUTestInstances[_InstanceIndex];
	Out.WSPosition = qmul(Inst.Rotation, _LSPosition) + Inst.Translation;
	Out.SSPosition = oGPUTestLStoSS(Out.WSPosition);
	Out.LSPosition = _LSPosition;
	Out.Color = GPUTestConstants.Color;
	return Out;
}

#endif
#endif
