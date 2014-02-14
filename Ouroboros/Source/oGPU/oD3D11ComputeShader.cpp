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
#include "oD3D11ComputeShader.h"
#include "oD3D11Device.h"
#include <oCore/windows/win_util.h>

using namespace ouro::d3d11;

oDEFINE_GPUDEVICE_CREATE(oD3D11, ComputeShader);
oBEGIN_DEFINE_GPUDEVICECHILD_CTOR(oD3D11, ComputeShader)
	, DebugName(_Desc.debug_name)
{
	*_pSuccess = false;
	if (!_Desc.cs)
	{
		oErrorSetLast(std::errc::invalid_argument, "A buffer of valid compute shader bytecode must be specified");
		return;
	}

	oD3D11DEVICE();
	oV(D3DDevice->CreateComputeShader(_Desc.cs, byte_code_size(_Desc.cs), nullptr, &ComputeShader));
	debug_name(ComputeShader, _Desc.debug_name);
	*_pSuccess = true;
}

void oD3D11ComputeShader::GetDesc(DESC* _pDesc) const threadsafe
{
	_pDesc->debug_name = DebugName;
	_pDesc->cs = nullptr;
}
