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
#pragma once
#ifndef oD3D11Texture_h
#define oD3D11Texture_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include <oPlatform/Windows/oD3D11.h>

// {75D2C3FA-A10C-48C2-ABCA-4CB84F4C447B}
oDECLARE_GPURESOURCE_IMPLEMENTATION(oD3D11, Texture, oGPU_TEXTURE, 0x75d2c3fa, 0xa10c, 0x48c2, 0xab, 0xca, 0x4c, 0xb8, 0x4f, 0x4c, 0x44, 0x7b)
{
	oDEFINE_GPURESOURCE_INTERFACE();
	//oDECLARE_GPURESOURCE_CTOR(oD3D11, Texture);
	oD3D11Texture(oGPUDevice* _pDevice, const DESC& _Desc, const char* _Name, bool* _pSuccess, ID3D11Texture2D* _pTexture = nullptr);

	oStd::intrusive_ptr<ID3D11Texture2D> Texture;
	oStd::intrusive_ptr<ID3D11ShaderResourceView> SRV;
	oStd::intrusive_ptr<ID3D11UnorderedAccessView> UAV;

	// Texture2 is used to emulate YUV aniso-sized formats. As DXGI specs more YUV
	// formats and HW supports them, this might get used less, but there are still
	// formats OOOii requires not on the posted DXGI roadmap, so this will be 
	// needed for the foreseeable future.
	oStd::intrusive_ptr<oD3D11Texture> Texture2;
};

#endif
