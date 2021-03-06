// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oGPU/resource.h>
#include "d3d_debug.h"
#include "d3d_util.h"
#include "dxgi_util.h"

using namespace ouro::gpu::d3d;

namespace ouro { namespace gpu {

DeviceContext* get_dc(command_list& cl);

char* resource::name(char* dst, size_t dst_size) const
{
	return ro ? debug_name(dst, dst_size, (View*)ro) : "uninitialized";
}

void* resource::get_buffer() const
{
	intrusive_ptr<Resource> r;
	((View*)ro)->GetResource(&r);
	return r;
}

void resource::set(command_list& cl, uint slot, uint num_resources, resource* const* resources)
{
	ShaderResourceView* srvs[D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT];
	for (uint i = 0; i < num_resources; i++)
		srvs[i] = (ShaderResourceView*)resources[i]->get_resource();
	set_srvs(get_dc(cl), slot, num_resources, srvs); 	
}

void resource::set(command_list& cl, uint slot)
{
	set_srvs(get_dc(cl), slot, 1, (ShaderResourceView* const*)&ro);
}

void resource::set_highest_mip(command_list& cl, float mip)
{
	intrusive_ptr<Resource> r;
	((View*)ro)->GetResource(&r);
	get_dc(cl)->SetResourceMinLOD(r, mip);
}

void resource::update(command_list& cl, uint subresource, const surface::const_mapped_subresource& src, const surface::box& region)
{
	intrusive_ptr<Resource> r;
	((View*)ro)->GetResource(&r);
	update_texture(get_dc(cl), true, r, subresource, src, region);
}

void set_viewports(command_list& cl, const uint2& default_dimensions, const viewport* oRESTRICT viewports, uint num_viewports)
{
	D3D11_VIEWPORT VPs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	if (viewports && num_viewports)
	{
		for (uint i = 0; i < num_viewports; i++)
		{
			const viewport& src = viewports[i];
			D3D11_VIEWPORT& dst = VPs[i];
			dst.TopLeftX = static_cast<float>(src.left == viewport::default_value ? 0 : src.left);
			dst.TopLeftY = static_cast<float>(src.top == viewport::default_value ? 0 : src.top);
			dst.Width = static_cast<float>(src.width == viewport::default_value ? default_dimensions.x : src.width);
			dst.Height = static_cast<float>(src.height == viewport::default_value ? default_dimensions.y : src.height);
			dst.MinDepth = src.min_depth;
			dst.MaxDepth = src.max_depth;
		}
	}

	else
	{
		num_viewports = 1;
		D3D11_VIEWPORT& dst = VPs[0];
		dst.TopLeftX = 0.0f;
		dst.TopLeftY = 0.0f;
		dst.Width = static_cast<float>(default_dimensions.x);
		dst.Height = static_cast<float>(default_dimensions.y);
		dst.MinDepth = 0.0f;
		dst.MaxDepth = 1.0f;
	}

	get_dc(cl)->RSSetViewports(num_viewports, VPs);
}

}}
