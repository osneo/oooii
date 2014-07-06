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
#include "d3d_debug.h"
#include <d3d11.h>
#include <oCore/windows/win_util.h>

namespace ouro {
	namespace gpu {
		namespace d3d {

void debug_name(Device* dev, const char* name)
{
	if (dev && name)
	{
		unsigned int CreationFlags = dev->GetCreationFlags();
		if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		{
			sstring Buffer(name); // if strings aren't the same size D3D issues a warning
			oV(dev->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(Buffer.capacity()), Buffer.c_str()));
		}
	}
}

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const Device* dev)
{
	unsigned int size = as_uint(_SizeofStrDestination);
	unsigned int CreationFlags = const_cast<Device*>(dev)->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(const_cast<Device*>(dev)->GetPrivateData(WKPDID_D3DDebugObjectName, &size, _StrDestination));
	else if (strlcpy(_StrDestination, "non-debug dev", _SizeofStrDestination) >= _SizeofStrDestination)
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

void debug_name(DeviceChild* child, const char* name)
{
	if (child && name)
	{
		intrusive_ptr<Device> dev;
		child->GetDevice(&dev);
		unsigned int CreationFlags = dev->GetCreationFlags();
		if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		{
			sstring Buffer(name); // if strings aren't the same size, D3D issues a warning
			oV(child->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(Buffer.capacity()), Buffer.c_str()));
		}
	}
}

char* debug_name(char* _StrDestination, size_t _SizeofStrDestination, const DeviceChild* child)
{
	unsigned int size = as_uint(_SizeofStrDestination);
	intrusive_ptr<Device> dev;
	const_cast<DeviceChild*>(child)->GetDevice(&dev);
	unsigned int CreationFlags = dev->GetCreationFlags();
	if (CreationFlags & D3D11_CREATE_DEVICE_DEBUG)
		oV(const_cast<DeviceChild*>(child)->GetPrivateData(WKPDID_D3DDebugObjectName, &size, _StrDestination));
	else if (strlcpy(_StrDestination, "non-debug dev child", _SizeofStrDestination) >= _SizeofStrDestination)
		oTHROW0(no_buffer_space);
	return _StrDestination;
}

int vtrace(ID3D11InfoQueue* iq, D3D11_MESSAGE_SEVERITY severity, const char* format, va_list args)
{
	xlstring buf;
	int len = vsnprintf(buf, format, args);
	if (len == -1)
		oTHROW0(no_buffer_space);
	iq->AddApplicationMessage(severity, buf);
	return len;
}

		} // namespace d3d
	} // namespace gpu
} // namespace ouro
