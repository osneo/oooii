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
// The inner implementation interface for oKinect.
#pragma once
#ifndef oKinectInternal_h
#define oKinectInternal_h

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinect.h>
#include <oConcurrency/mutex.h>
#include <oConcurrency/condition_variable.h>
#include "oKinectSkeletonStream.h"
#include <NuiApi.h>

struct oKinectImpl : oKinect
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	
	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oKinectImpl(const oKINECT_DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess);
	~oKinectImpl();

	void GetDesc(oKINECT_DESC* _pDesc) const threadsafe override;
	int2 GetDimensions(oKINECT_FRAME_TYPE _Type) const threadsafe override;
	oGUI_INPUT_DEVICE_STATUS GetStatus() const threadsafe override;
	void SetPitch(int _Degrees) threadsafe override;

	bool MapRead(oKINECT_FRAME_TYPE _Type, oSURFACE_DESC* _pDesc, oSURFACE_CONST_MAPPED_SUBRESOURCE* _pDestination) const threadsafe override;
	void UnmapRead(oKINECT_FRAME_TYPE _Type) const threadsafe override;

	bool GetSkeletonByIndex(int _PlayerIndex, oGUI_BONE_DESC* _pSkeleton) const threadsafe override;
	bool GetSkeletonByID(unsigned int _ID, oGUI_BONE_DESC* _pSkeleton) const threadsafe override;

	// @oooii-tony: These should become threadsafe...

	// Soft-restarts this object based off the same Desc as originally passed to
	// oKinectCreate.
	bool Reinitialize();

	// Shuts down only the directly INuiSensor-related objects without disrupting
	// the conceptual state. Use Reinitialize to restore full functionality.
	void Shutdown();

protected:
	oRef<INuiSensor> NUISensor;
	oRef<threadsafe oWindow> Window;
	oKINECT_DESC Desc;
	oConcurrency::condition_variable PitchCV;
	oConcurrency::mutex PitchMutex;
	oStd::thread PitchThread;
	oStd::thread EventThread;
	oRefCount RefCount;
	bool Running;
	int NewPitch;

	enum oKINECT_EVENT
	{
		oKINECT_EVENT_WAKEUP,
		oKINECT_EVENT_COLOR_FRAME,
		oKINECT_EVENT_DEPTH_FRAME,
		oKINECT_EVENT_SKELETON,

		oKINECT_EVENT_COUNT,
	};

	HANDLE hEvents[oKINECT_EVENT_COUNT];
	HANDLE hColorStream;
	HANDLE hDepthStream;

	oRef<threadsafe oSurface> Color;
	oRef<threadsafe oSurface> Depth;

	oKinectSkeletonStream Skeletons;

	void OnPitch();
	void OnEvent();

	void CacheNextSkeletonFrame(INuiSensor* _pSensor);
	void CheckTrackingTimeouts();
};

#endif // oHAS_KINECT_SDK
#endif