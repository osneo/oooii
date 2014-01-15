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
#include "oKinectManager.h"

#ifdef oHAS_KINECT_SDK

#include <oKinect/oKinectUtil.h>
#include <oPlatform/oSingleton.h>
#include "oGUI/Windows/oWinWindowing.h"
#include "oWinKinect10.h"
#include "oKinectInternal.h"

#include <oCore/windows/win_error.h>

using namespace ouro;
using namespace std;

// {1ECB3B76-4BB7-4B7A-B413-3BE946A44128}
const oGUID oKinectManager::GUID = { 0x1ecb3b76, 0x4bb7, 0x4b7a, { 0xb4, 0x13, 0x3b, 0xe9, 0x46, 0xa4, 0x41, 0x28 } };

oSINGLETON_REGISTER(oKinectManager);

oKinectManager::oKinectManager()
	: WMInputDeviceChange(0)
	, CurrentDeviceInstanceName(0)
{
	oWinKinect10::Singleton().SafeNuiSetDeviceStatusCallback(StatusProc, this);
	WMInputDeviceChange = oWinGetNativeRegisteredMessage(oWM_INPUT_DEVICE_CHANGE);
}

oKinectManager::~oKinectManager()
{
	oASSERT(Kinects.empty(), "");
	oWinKinect10::Singleton().SafeNuiSetDeviceStatusCallback(nullptr, nullptr);
}

void oKinectManager::Register(threadsafe oKinect* _pKinect)
{
	oKINECT_DESC kd;
	_pKinect->GetDesc(&kd);
	lock_t lock(KinectsMutex);
	oASSERT(find(Kinects, _pKinect) == std::end(Kinects), "");
	Kinects.push_back(_pKinect);
}

void oKinectManager::Unregister(threadsafe oKinect* _pKinect)
{
	oKINECT_DESC kd;
	_pKinect->GetDesc(&kd);
	lock_t lock(KinectsMutex);
	find_and_erase(Kinects, _pKinect);
}

void oKinectManager::StatusProc(HRESULT _hrStatus, const OLECHAR* _InstanceName, const OLECHAR* _UniqueDeviceName, void* _pThis)
{
	ouro::input::status hrStatus = oKinectStatusFromHR(_hrStatus);
	sstring InstanceName(_InstanceName);
	sstring UniqueDeviceName(_UniqueDeviceName);
	static_cast<oKinectManager*>(_pThis)->OnStatus(hrStatus, InstanceName, UniqueDeviceName);
}

void oKinectManager::NotifyStatus(const char* _InstanceName, ouro::input::status _Status)
{
	// Hope we don't overwrite ourselves. Hope.
	DeviceInstanceNames[CurrentDeviceInstanceName] = _InstanceName;
	::PostMessage(HWND_BROADCAST, WMInputDeviceChange, MAKEWPARAM(ouro::input::skeleton, _Status), (LPARAM)DeviceInstanceNames[CurrentDeviceInstanceName].c_str());
	::PostMessage(HWND_BROADCAST, WMInputDeviceChange, MAKEWPARAM(ouro::input::voice, _Status), (LPARAM)DeviceInstanceNames[CurrentDeviceInstanceName].c_str());
	CurrentDeviceInstanceName = (CurrentDeviceInstanceName + 1) % DeviceInstanceNames.size();
}

void oKinectManager::OnStatus(ouro::input::status _Status, const char* _InstanceName, const char* _UniqueDeviceName)
{
	intrusive_ptr<threadsafe oKinect> StatusChanger;
	oKINECT_DESC kd;
	{
		lock_t lock(KinectsMutex);
		for (auto it = std::begin(Kinects); it != std::end(Kinects); ++it)
		{
			(*it)->GetDesc(&kd);
			if (!strcmp(kd.ID, _InstanceName))
			{
				StatusChanger = *it;
				break;
			}
		}
	}

	//oTRACE("%segistered oKinect Ptr=0x%p Inst=%s UID=%s: %s", StatusChanger ? "R" : "Unr", StatusChanger.c_ptr(), _InstanceName, _UniqueDeviceName, ouro::as_string(_Status));

	threadsafe oKinectImpl* pImpl = static_cast<threadsafe oKinectImpl*>(StatusChanger.c_ptr());

	// Make the APIs threadsafe...
	oKinectImpl* pUnsafeImpl = thread_cast<oKinectImpl*>(pImpl);
		
	switch (_Status)
	{
		case ouro::input::ready:
		{
			if (pUnsafeImpl)
				oVB(pUnsafeImpl->Reinitialize());

			NotifyStatus(_InstanceName, _Status);
			break;
		}

		case ouro::input::not_connected:
		{
			NotifyStatus(_InstanceName, _Status);
	
			if (pUnsafeImpl)
				pUnsafeImpl->Shutdown();

			break;
		}

		default:
			NotifyStatus(_InstanceName, _Status);
			break;
	}
}

#endif // oHAS_KINECT_SDK
