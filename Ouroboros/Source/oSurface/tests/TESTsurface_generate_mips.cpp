// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/codec.h>
#include <oBase/colors.h>
#include <oString/path.h>
#include <oBase/throw.h>

#include "../../test_services.h"

namespace ouro { 
	namespace tests {

static const surface::filter kFilter = surface::filter::lanczos2;

static surface::image surface_load(test_services& _Services, const path& _Path)
{
	scoped_allocation b = _Services.load_buffer(_Path);
	return surface::decode(b, b.size());
}

static surface::image make_test_1d(int _Width)
{
	surface::info si;
	si.dimensions = int3(_Width, 1, 1);
	si.format = surface::format::b8g8r8a8_unorm;
	surface::image s(si);

	{
		surface::lock_guard lock(s);
		static const color sConsoleColors[] = { black, navy, green, teal, maroon, purple, olive, silver, gray, blue, lime, aqua, red, fuchsia, yellow, white };
		color* texture1Ddata = (color*)lock.mapped.data;
		for (uint i = 0; i < si.dimensions.x; i++)
			texture1Ddata[i] = sConsoleColors[i % oCOUNTOF(sConsoleColors)];
	}

	return s;
}

static surface::image load_test_cube(test_services& _Services)
{
	const char* face_paths[6] =
	{
		"Test/Textures/CubePosX.png",
		"Test/Textures/CubeNegX.png",
		"Test/Textures/CubePosY.png",
		"Test/Textures/CubeNegY.png",
		"Test/Textures/CubePosZ.png",
		"Test/Textures/CubeNegZ.png",
	};

	auto image = surface_load(_Services, face_paths[0]);

	auto si = image.get_info();
	si.array_size = oCOUNTOF(face_paths);
	surface::image cube_image(si);
	cube_image.copy_from(0, image, 0);

	for (int i = 1; i < oCOUNTOF(face_paths); i++)
	{
		image = surface_load(_Services, face_paths[i]);
		int subresource = surface::calc_subresource(0, i, 0, 0, si.array_size);
		cube_image.copy_from(subresource, image, 0);
	}

	return cube_image;
}

static void test_mipchain(test_services& _Services, const surface::image& _Image, surface::filter _Filter, surface::mip_layout _Layout, int _StartIndex)
{
	auto si = _Image.get_info();
	si.mip_layout = _Layout;
	surface::image mipchain(si);
	mipchain.clear();

	int nSlices = max(si.array_size, si.dimensions.z);
	int nMips = surface::num_mips(si.mip_layout, si.dimensions);

	if (si.dimensions.z != 1)
	{
		int subresource = surface::calc_subresource(0, 0, 0, nMips, si.array_size);
		surface::box region;
		region.right = si.dimensions.x;
		region.bottom = si.dimensions.y;
		surface::shared_lock lock(_Image, subresource);
		for (int i = 0; i < nSlices; i++)
		{
			region.front = i;
			region.back = i + 1;

			mipchain.update_subresource(subresource, region, lock.mapped);
			lock.mapped.data = byte_add(lock.mapped.data, lock.mapped.depth_pitch);
		}
	}

	else
	{
		uint nSlices = max(1u, si.array_size);
		for (uint i = 0; i < nSlices; i++)
		{
			int DstSubresource = surface::calc_subresource(0, i, 0, nMips, nSlices);
			int SrcSubresource = surface::calc_subresource(0, i, 0, 0, nSlices);
			mipchain.copy_from(DstSubresource, _Image, SrcSubresource);
		}
	}

	mipchain.generate_mips(_Filter);
	mipchain.flatten();
	_Services.check(mipchain, _StartIndex);
}

static void test_mipchain_layouts(test_services& _Services, const surface::image& _Image, surface::filter _Filter, int _StartIndex)
{
	test_mipchain(_Services, _Image, _Filter, surface::mip_layout::tight, _StartIndex);
	test_mipchain(_Services, _Image, _Filter, surface::mip_layout::below, _StartIndex+1);
	test_mipchain(_Services, _Image, _Filter, surface::mip_layout::right, _StartIndex+2);
}

void TESTsurface_generate_mips(test_services& _Services)
{
	auto image = make_test_1d(227); // 1D NPOT
	test_mipchain_layouts(_Services, image, kFilter, 0);

	image = make_test_1d(512); // 1D POT
	test_mipchain_layouts(_Services, image, kFilter, 3);

	image = surface_load(_Services, "Test/Textures/lena_npot.png"); // 2D NPOT
	test_mipchain_layouts(_Services, image, kFilter, 6);

	image = surface_load(_Services, "Test/Textures/lena_1.png"); // 2D POT
	test_mipchain_layouts(_Services, image, kFilter, 9);

	{
		image = surface_load(_Services, "Test/Textures/lena_npot.png"); // 2D NPOT
		const surface::image* images[5] = { &image, &image, &image, &image, &image };
		surface::image image3d;
		image3d.initialize_3d(images, oCOUNTOF(images));
		test_mipchain_layouts(_Services, image3d, kFilter, 12);
	}
	{
		image = surface_load(_Services, "Test/Textures/lena_1.png"); // 2D POT
		const surface::image* images[5] = { &image, &image, &image, &image, &image };
		surface::image image3d;
		image3d.initialize_3d(images, oCOUNTOF(images));
		test_mipchain_layouts(_Services, image3d, kFilter, 15);
	}

	image = load_test_cube(_Services);
	test_mipchain_layouts(_Services, image, kFilter, 18);
}

	}
}
