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
#ifndef oD3D11CommandList_h
#define oD3D11CommandList_h

#include <oGPU/oGPU.h>
#include "oGPUCommon.h"
#include "oD3D11RenderTarget.h"
#include "oD3D11Device.h"
#include <oPlatform/Windows/oD3D11.h>

ID3D11Resource* oD3D11GetSubresource(oGPUResource* _pResource, int _Subresource, int* _pD3DSubresourceIndex);

oDECLARE_GPUDEVICECHILD_IMPLEMENTATION(oD3D11, CommandList)
{
	oDEFINE_CONST_GETDESC_INTERFACE(Desc, threadsafe);
	oDEFINE_GPUDEVICECHILD_INTERFACE_EXPLICIT_QI();
	oDECLARE_GPUDEVICECHILD_CTOR(oD3D11, CommandList);
	~oD3D11CommandList();

	void Begin() override;
	void End() override;
	void Flush() override;
	void Reset() override;
	void Reserve(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE* _pMappedSubresource) override;
	void Commit(oGPUResource* _pResource, int _Subresource, oSURFACE_MAPPED_SUBRESOURCE& _Source, const oGPU_BOX& _Subregion = oGPU_BOX()) override;
	void Copy(oGPUResource* _pDestination, oGPUResource* _pSource) override;
	void Copy(oGPUBuffer* _pDestination, int _DestinationOffsetBytes, oGPUBuffer* _pSource, int _SourceOffsetBytes, int _SizeBytes) override;
	void CopyCounter(oGPUBuffer* _pDestination, uint _DestinationAlignedOffset, oGPUBuffer* _pUnorderedSource) override;
	void SetCounters(int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pValues) override;
	void SetSamplers(int _StartSlot, int _NumStates, const oGPU_SAMPLER_STATE* _pSamplerState) override;
	void SetShaderResources(int _StartSlot, int _NumResources, const oGPUResource* const* _ppResources) override;
	void SetBuffers(int _StartSlot, int _NumBuffers, const oGPUBuffer* const* _ppConstants) override;
	void SetIndexBuffer(const oGPUBuffer* _pIndexBuffer) override;
	void SetVertexBuffers(int _StartSlot, int _NumVertexBuffers, const oGPUBuffer* const* _ppVertexBuffers, uint _StartVertEx) override;

	void SetRenderTargetAndUnorderedResources(oGPURenderTarget* _pRenderTarget, int _NumViewports, const oAABoxf* _pViewports, bool _SetForDispatch, int _UnorderedResourcesStartSlot, int _NumUnorderedResources, oGPUResource** _ppUnorderedResources, uint* _pInitialCounts = nullptr) override;
	void SetPipeline(const oGPUPipeline* _pPipeline) override;
	void SetSurfaceState(oGPU_SURFACE_STATE _State) override;
	void SetBlendState(oGPU_BLEND_STATE _State) override;
	void SetDepthStencilState(oGPU_DEPTH_STENCIL_STATE _State) override;
	void Clear(oGPURenderTarget* _pRenderTarget, oGPU_CLEAR _Clear) override;
	void oD3D11CommandList::Draw(uint _StartPrimitive, uint _NumPrimitives, uint _StartInstance = oInvalid, uint _NumInstances = oInvalid) override;
	void Draw(const oGPUMesh* _pMesh, int _RangeIndex, const oGPUInstanceList* _pInstanceList = nullptr) override;
	void Draw(const oGPULineList* _pLineList) override;
	void Draw(uint _VertexCount) override;
	void Draw(oGPUBuffer* _pDrawArgs, int _AlignedByteOffsetForArgs) override;
	void DrawSVQuad(uint _NumInstances = 1) override;
	bool GenerateMips(oGPURenderTarget* _pRenderTarget) override;
	void ClearI(oGPUResource* _pUnorderedResource, const uint4 _Values) override;
	void ClearF(oGPUResource* _pUnorderedResource, const float4 _Values) override;

	void Dispatch(oGPUComputeShader* _pComputeShader, const int3& _ThreadGroupCount) override;
	void Dispatch(oGPUComputeShader* _pComputeShader, oGPUBuffer* _pThreadGroupCountBuffer, int _AlignedByteOffsetToThreadGroupCount) override;

	oRef<ID3D11DeviceContext> Context;
	oRef<ID3D11CommandList> CommandList;
	DESC Desc;
	D3D_PRIMITIVE_TOPOLOGY PrimitiveTopology;
	bool IndicesSet;

	// Shortcut to a bunch of typecasting
	// This thread_cast is safe because oD3D11CommandList is single-threaded
	// and most access is to get at common/safe resources
	inline oD3D11Device* D3DDevice() { return thread_cast<oD3D11Device*>(static_cast<threadsafe oD3D11Device*>(Device.c_ptr())); }

private:
	void CSSetState(oGPUComputeShader* _pComputeShader, int _NumUnorderedResources, const oGPUResource* const* _ppUnorderedResources, uint* _pInitialCounts);
};

#endif