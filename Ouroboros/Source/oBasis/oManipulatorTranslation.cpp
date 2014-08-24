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
#include "oManipulatorTranslation.h"
#include <oCompute/oComputeConstants.h>

const float TRANSLATION_CAP_OFFSET = 0.1f;
const float TRANSLATION_CAP_SCALE  = 0.001f;
const float TRANSLATION_CENTER_SCALE = 0.25f;

using namespace ouro;

oManipulatorTranslation::oManipulatorTranslation(const DESC& _Desc, bool *_pSuccess) : oManipulatorBase(_Desc, _pSuccess)
{
	if(!(*_pSuccess)) //base failed to construct
		return;
	*_pSuccess = false;
	Clip[X].resize(2);
	Clip[Y].resize(2);
	Clip[Z].resize(2);
	Clip[SCREEN].resize(4);
	*_pSuccess = true;
}

void oManipulatorTranslation::BeginManipulation(AXIS _Axis, const float2& _ScreenPosition)
{
	Picking = true;
	PickAxis = _Axis;
	StartTranslation = _ScreenPosition;
	CurrentTranslation = _ScreenPosition;
}

void oManipulatorTranslation::GetTransform(float4x4 *_pTransform) const
{
	*_pTransform = World*Transform;
}

float3 oManipulatorTranslation::CalcTranslation(float3 _axis)
{
	float3 center = View*float3(0,0,0);
	float3 axis = View*_axis;

	float proj = center.z;
	axis = axis - center;
	axis = normalize(axis);

	float3 mouseVector = float3(CurrentTranslation.x - StartTranslation.x, CurrentTranslation.y - StartTranslation.y, 0);
	if(length(mouseVector) == 0)
		return float3(0,0,0);
	mouseVector *= proj;
	mouseVector = InvProj*mouseVector;
	mouseVector.z = 0.0;
	float3 move = dot(axis,mouseVector)*axis;
	move += center;
	move = invert(View)*move;
	return move;
}

bool oManipulatorTranslation::CalcGeometry(AXIS _Axis)
{
	oASSERT(Lines[_Axis].size() == 2,"CalcGeometry assumes 2 vertices for line, and 4 for the pick rect");

	if(Project(Lines[_Axis]))
		return true;

	float zdirection = Lines[_Axis][1].z - Lines[_Axis][0].z;
	Lines[_Axis][0].z = static_cast<float>(0.01);
	Lines[_Axis][1].z = static_cast<float>(0.01);

	//handle screen pick rect. get a perp vector from line and create the 4 corners of the rect
	float4 normal = Lines[_Axis][1]-Lines[_Axis][0];
	float tempf = normal.x;
	normal.x = -normal.y;
	normal.y = tempf;
	normal.z = 0;
	normal.w = 0;
	normal = normalize(normal) * Desc.PickWidth;

	PickGeometry[_Axis].resize(4);
	if(zdirection > 0) //so z clipping between the several pick rects, x, y, and z, work a little better. i.e. which will have priority on getting picked
	{
		PickGeometry[_Axis][0] = Lines[_Axis][0]+normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][2] = Lines[_Axis][0]-normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][1] = Lines[_Axis][1]+normal+float4(0,0,0.02f,0);
		PickGeometry[_Axis][3] = Lines[_Axis][1]-normal+float4(0,0,0.02f,0);
	}
	else
	{
		PickGeometry[_Axis][0] = Lines[_Axis][0]+normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][2] = Lines[_Axis][0]-normal+float4(0,0,0.01f,0);
		PickGeometry[_Axis][1] = Lines[_Axis][1]+normal+float4(0,0,0,0);
		PickGeometry[_Axis][3] = Lines[_Axis][1]-normal+float4(0,0,0,0);
	}

	UnProject(Lines[_Axis]);
	UnProject(PickGeometry[_Axis]);
	return false;
}

void oManipulatorTranslation::CalcCap(AXIS _Axis, float4 vert, const float4x4 &_capRotation)
{
	vert = WorldTransViewProj*vert;

	float scale = static_cast<float>(1.0 - vert.z/vert.w);
	vert /= vert.w;
	vert.z = TRANSLATION_CAP_OFFSET;

	UnProject(vert);

	CapTransforms[_Axis] = _capRotation*make_scale(TRANSLATION_CAP_SCALE)*make_translation(vert.xyz());
}

void oManipulatorTranslation::UpdateImpl()
{
	Transform = oIDENTITY4x4;
	if(Picking)
	{
		static float3 axes[NUM_AXES] = {float3(1,0,0),float3(0,1,0),float3(0,0,1)};
		if(PickAxis == X || PickAxis == Y || PickAxis == Z)
		{
			float3 axis;

			axis = axes[PickAxis];
			if(Desc.Mode == DESC::OBJECT)
				axis = WorldRotation*axis;

			Transform = make_translation(CalcTranslation(axis.xyz()));
		}
		else if(PickAxis == SCREEN)
		{
			float3 moveX = CalcTranslation(axes[X]);
			float3 moveY = CalcTranslation(axes[Y]);
			float3 moveZ = CalcTranslation(axes[Z]);
			Transform = make_translation((moveX+moveY+moveZ));
		}
	}

	if(Desc.Mode == DESC::WORLD)
		WorldTransViewProj = WorldTranslation*Transform*View*Proj;
	else
		WorldTransViewProj = WorldRotation*WorldTranslation*Transform*View*Proj;

	Lines[X].resize(2);
	Lines[X][0] = float4(0,0,0,1);
	Lines[X][1] = float4(Desc.ManipularScale,0,0,1);
	Lines[Y].resize(2);
	Lines[Y][0] = float4(0,0,0,1);
	Lines[Y][1] = float4(0,Desc.ManipularScale,0,1);
	Lines[Z].resize(2);
	Lines[Z][0] = float4(0,0,0,1);
	Lines[Z][1] = float4(0,0,Desc.ManipularScale,1);

	Clipped[X] = CalcGeometry(X);
	Clipped[Y] = CalcGeometry(Y);
	Clipped[Z] = CalcGeometry(Z);

	float4x4 capXRot = make_rotation(radians(static_cast<float>(90.0)), float3(0,1,0));
	if(Desc.Mode == DESC::OBJECT)
		capXRot = capXRot * WorldRotation;
	float4x4 capYRot = make_rotation(radians(static_cast<float>(-90.0)), float3(1,0,0));
	if(Desc.Mode == DESC::OBJECT)
		capYRot = capYRot * WorldRotation;
	float4x4 capZRot = oIDENTITY4x4;
	if(Desc.Mode == DESC::OBJECT)
		capZRot = capZRot * WorldRotation;

	CalcCap(X, float4(Desc.ManipularScale,0,0,1), capXRot);
	CalcCap(Y, float4(0,Desc.ManipularScale,0,1), capYRot);
	CalcCap(Z, float4(0,0,Desc.ManipularScale,1), oIDENTITY4x4);

	Lines[SCREEN].resize(4);
	float4 center = float4(0,0,0,1);
	center = WorldTransViewProj*center;
	float centerBoxRadius = TRANSLATION_CENTER_SCALE*(Desc.ManipularScale/center.w);
	if(center.w > 0)
		Clipped[SCREEN] = false;
	else
		Clipped[SCREEN] = true;
	if(!Clipped[SCREEN])
	{
		center /= center.w;
		center.z = static_cast<float>(0.01);
		Lines[SCREEN][0] = center+float4(-centerBoxRadius, -centerBoxRadius, 0, 0);
		Lines[SCREEN][1] = center+float4(-centerBoxRadius, centerBoxRadius, 0, 0);
		Lines[SCREEN][2] = center+float4(centerBoxRadius, centerBoxRadius, 0, 0);
		Lines[SCREEN][3] = center+float4(centerBoxRadius, -centerBoxRadius, 0, 0);
		UnProject(Lines[SCREEN]);
	}

	PickGeometry[SCREEN].resize(4);
	PickGeometry[SCREEN][0] = center+float4(-centerBoxRadius, -centerBoxRadius, 0, 0);
	PickGeometry[SCREEN][1] = center+float4(centerBoxRadius, -centerBoxRadius, 0, 0);
	PickGeometry[SCREEN][2] = center+float4(-centerBoxRadius, centerBoxRadius, 0, 0);
	PickGeometry[SCREEN][3] = center+float4(centerBoxRadius, centerBoxRadius, 0, 0);
	UnProject(PickGeometry[SCREEN]);
}