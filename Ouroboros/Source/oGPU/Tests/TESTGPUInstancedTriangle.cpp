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
#include <oPlatform/oTest.h>
#include "oGPUTestCommon.h"
#include <oGPU/oGPUUtil.h>

#include <oBasis/oMath.h>

using namespace ouro;

static const int sSnapshotFrames[] = { 1, 23, 46 };
static const bool kIsDevMode = false;

struct GPU_InstancedTriangle_App : public oGPUTestApp
{
	GPU_InstancedTriangle_App() : oGPUTestApp("GPU_InstancedTriangle", kIsDevMode, sSnapshotFrames) {}

	bool Initialize() override
	{
		PrimaryRenderTarget->SetClearColor(almost_black);

		oGPUBuffer::DESC DCDesc;
		DCDesc.struct_byte_size = sizeof(oGPUTestConstants);
		if (!Device->CreateBuffer("TestConstants", DCDesc, &TestConstants))
			return false;

		oGPUPipeline::DESC pld = oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE);

		gpu::buffer_info i;
		i.type = gpu::buffer_type::constant;
		i.array_size = 2;
		i.struct_byte_size = sizeof(oGPU_TEST_INSTANCE);

		if (!Device->CreateBuffer("Instances", i, &InstanceList))
			return false;

		if (!Device->CreatePipeline(pld.debug_name, pld, &Pipeline))
			return false;

		pld = oGPUTestGetPipeline(oGPU_TEST_TRANSFORMED_WHITE);

		Mesh = ouro::gpu::make_first_triangle(Device);

		return true;
	}
	
	bool Render() override
	{
		float4x4 V = make_lookat_lh(float3(0.0f, 0.0f, -3.5f), oZERO3, float3(0.0f, 1.0f, 0.0f));

		oGPURenderTarget::DESC RTDesc;
		PrimaryRenderTarget->GetDesc(&RTDesc);
		float4x4 P = make_perspective_lh(oDEFAULT_FOVY_RADIANS, RTDesc.dimensions.x / oCastAsFloat(RTDesc.dimensions.y), 0.001f, 1000.0f);

		{
			ouro::surface::mapped_subresource msr;
			CommandList->Reserve(InstanceList, 0, &msr);
			oGPU_TEST_INSTANCE* pInstances = (oGPU_TEST_INSTANCE*)msr.data;
			{
				pInstances[0].Translation = float3(-0.5f, 0.5f, 0.0f);
				pInstances[1].Translation = float3(0.5f, -0.5f, 0.0f);

				float rotationStep = Device->GetFrameID() * 1.0f;
				pInstances[0].Rotation = make_quaternion(float3(radians(rotationStep) * 0.75f, radians(rotationStep), radians(rotationStep) * 0.5f));
				pInstances[1].Rotation = make_quaternion(float3(radians(rotationStep) * 0.5f, radians(rotationStep), radians(rotationStep) * 0.75f));
			}
			CommandList->Commit(InstanceList, 0, msr);
		}

		CommandList->Begin();

		ouro::gpu::commit_buffer(CommandList, TestConstants, oGPUTestConstants(oIDENTITY4x4, V, P, white));

		CommandList->Clear(PrimaryRenderTarget, ouro::gpu::clear_type::color_depth_stencil);
		CommandList->SetBlendState(ouro::gpu::blend_state::opaque);
		CommandList->SetDepthStencilState(ouro::gpu::depth_stencil_state::test_and_write);
		CommandList->SetSurfaceState(ouro::gpu::surface_state::two_sided);
		
		const oGPUBuffer* CBs[2] = { TestConstants.c_ptr(), InstanceList.c_ptr() };
		
		CommandList->SetBuffers(0, CBs);
		CommandList->SetPipeline(Pipeline);
		CommandList->SetRenderTarget(PrimaryRenderTarget);
		
		Mesh->draw(CommandList);

		CommandList->End();
		return true;
	}

private:
	intrusive_ptr<oGPUPipeline> Pipeline;
	intrusive_ptr<oGPUBuffer> InstanceList;
	std::shared_ptr<ouro::gpu::util_mesh> Mesh;
	intrusive_ptr<oGPUBuffer> TestConstants;
};

oDEFINE_GPU_TEST(GPU_InstancedTriangle)
