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
#include <oBase/surface.h>
#include <oBase/throw.h>

//#include <oBase/macros.h>

namespace ouro {
	namespace tests {

static void TESTsurface_row_pitch(int _Depth, int _ArraySize)
{
	// This tests whether the row pitch remains the same per mip level regardless 
	// of depth or number of slices
	surface::info inf;
	inf.dimensions = int3(512, 512, _Depth);
	inf.array_size = _ArraySize;
	inf.format = surface::r8g8b8a8_unorm;

	int RowPitches[10] = { 0x800, 0x400, 0x200, 0x100, 0x80, 0x40, 0x20, 0x10, 0x8, 0x4 };

	inf.layout = surface::image;
	oFORI(mipLevel, RowPitches)
	{
		oCHECK(surface::row_pitch(inf, 0) == RowPitches[mipLevel], "Test_oSurfaceMipCalcRowPitch(%d, %d) surface::image failed", _Depth, _ArraySize);
		inf.dimensions = surface::dimensions_npot(inf.format, inf.dimensions, 1);
	}
	inf.dimensions = int3(512, 512, _Depth);

	inf.layout = surface::tight;
	oFORI(mipLevel, RowPitches)
		oCHECK(surface::row_pitch(inf, (int)mipLevel) == RowPitches[mipLevel], "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_TIGHT failed", _Depth, _ArraySize);

	inf.layout = surface::below;
	oFORI(mipLevel, RowPitches)
		oCHECK(surface::row_pitch(inf, (int)mipLevel) == RowPitches[0], "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_BELOW failed", _Depth, _ArraySize);

	inf.layout = surface::right;
	oFORI(mipLevel, RowPitches)
		oCHECK(surface::row_pitch(inf, (int)mipLevel) == (RowPitches[0]+RowPitches[1]), "Test_oSurfaceMipCalcRowPitch(%d, %d) oSURFACE_LAYOUT_RIGHT failed", _Depth, _ArraySize);
}

static void TESTsurface_depth_pitch(int _Depth, int _ArraySize)
{
	// This tests whether the depth pitch remains the same per mip level regardless of depth or number of slices
	surface::info inf;
	inf.dimensions = int3(512,512,_Depth);
	inf.array_size = _ArraySize;
	inf.format = surface::r8g8b8a8_unorm;

	int DepthPitches[10] = { 0x100000, 0x40000, 0x10000, 0x4000, 0x1000, 0x400, 0x100, 0x40, 0x10, 0x4 };

	inf.layout = surface::image;
	oFORI(mipLevel, DepthPitches)
	{
		oCHECK(surface::depth_pitch(inf, 0) == DepthPitches[mipLevel], "Test_oSurfaceMipCalcDepthPitch() surface::image failed");
		inf.dimensions = surface::dimensions_npot(inf.format, inf.dimensions, 1);
	}
	inf.dimensions = int3(512,512,_Depth);

	inf.layout = surface::tight;
	oFORI(mipLevel, DepthPitches)
		oCHECK(surface::depth_pitch(inf, (int)mipLevel) == DepthPitches[mipLevel], "Test_oSurfaceMipCalcDepthPitch() oSURFACE_LAYOUT_TIGHT failed");

	inf.layout = surface::below;
	oFORI(mipLevel, DepthPitches)
		oCHECK(surface::depth_pitch(inf, (int)mipLevel) == DepthPitches[mipLevel] << mipLevel, "surface::depth_pitch() oSURFACE_LAYOUT_BELOW failed");

	inf.layout = surface::right;
	oFORI(mipLevel, DepthPitches)
		oCHECK(surface::depth_pitch(inf, (int)mipLevel) == (DepthPitches[mipLevel] << mipLevel) + ((DepthPitches[mipLevel] >> 1) << mipLevel), "surface::depth_pitch() oSURFACE_LAYOUT_RIGHT failed");
}

static void TESTsurface_slice_pitch(int _ArraySize)
{
	surface::info inf;
	inf.dimensions = int3(512,512,1);
	inf.array_size = _ArraySize;
	inf.format = surface::r8g8b8a8_unorm;

	if (_ArraySize > 1)
	{
		// This tests whether the slice pitch remains the same regardless of number of slices
		surface::info inf1 = inf;
		inf1.array_size = 1;

		inf.layout = surface::image;
		oCHECK(surface::slice_pitch(inf) == 0x00100000, "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == surface::depth_pitch(inf), "surface::slice_pitch() failed");
		
		inf.layout = surface::tight;
		oCHECK(surface::slice_pitch(inf) == 0x00155800, "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == (surface::total_size(inf) / _ArraySize), "surface::slice_pitch()==(surface::total_size()/slices) failed");

		inf.layout = surface::below;
		oCHECK(surface::slice_pitch(inf) == 0x00180000, "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == (surface::total_size(inf) / _ArraySize), "surface::slice_pitch()==(surface::total_size()/slices) failed");

		inf.layout = surface::right;
		oCHECK(surface::slice_pitch(inf) == 0x00180000, "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == (surface::total_size(inf) / _ArraySize), "surface::slice_pitch()==(surface::total_size()/slices) failed");
		return;
	}

	// This tests whether the slice pitch is the same as the depth pitch multiplied by the depth
	inf.layout = surface::image;
	for(int depthLevel=0; depthLevel<10; depthLevel++)
	{
		inf.dimensions.z = (int)depthLevel + 1;
		oCHECK(surface::slice_pitch(inf) == (surface::depth_pitch(inf, 0) * inf.dimensions.z), "surface::slice_pitch()==(surface::depth_pitch()*depth) failed");
		oCHECK(surface::slice_pitch(inf) == surface::total_size(inf), "surface::slice_pitch()==surface::total_size() failed");
	}

	// The rest below tests the slice pitches for a number of depth levels, and 3d textures don't support more than one slice
	inf.layout = surface::tight;
	int SlicePitches[15] = { 0x00155800, 0x00255800, 0x00355800, 0x00495800, 0x00595800, 0x006d5800, 0x007d5800, 0x00925800, 0x00a25800, 0x00b65800, 0x00c65800, 0x00db5800, 0x00eb5800, 0x00ff5800, 0x010f5800 };
	oFORI(depthLevel, SlicePitches)
	{
		inf.dimensions.z = (int)depthLevel + 1;
		oCHECK(surface::slice_pitch(inf) == SlicePitches[depthLevel], "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == surface::total_size(inf), "surface::slice_pitch()==surface::total_size() failed");
	}

	inf.layout = surface::below;
	int SlicePitchesVertical[15] = { 0x00180000, 0x00280000, 0x00380000, 0x00500000, 0x00600000, 0x00780000, 0x00880000, 0x00a00000, 0x00b00000, 0x00c80000, 0x00d80000, 0x00f00000, 0x01000000, 0x01180000, 0x01280000 };
	oFORI(depthLevel, SlicePitchesVertical)
	{
		inf.dimensions.z = (int)depthLevel + 1;
		oCHECK(surface::slice_pitch(inf) == SlicePitchesVertical[depthLevel], "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == surface::total_size(inf), "surface::slice_pitch()==surface::total_size() failed");
	}

	inf.layout = surface::right;
	int SlicePitchesHorizontal[15] = { 0x00180000, 0x00300000, 0x00480000, 0x00600000, 0x00780000, 0x00900000, 0x00a80000, 0x00c00000, 0x00d80000, 0x00f00000, 0x01080000, 0x01200000, 0x01380000, 0x01500000, 0x01680000 };
	oFORI(depthLevel, SlicePitchesHorizontal)
	{
		inf.dimensions.z = (int)depthLevel + 1;
		oCHECK(surface::slice_pitch(inf) == SlicePitchesHorizontal[depthLevel], "surface::slice_pitch() failed");
		oCHECK(surface::slice_pitch(inf) == surface::total_size(inf), "surface::slice_pitch()==surface::total_size() failed");
	}
}

void TESTsurface()
{
	for (int i=1; i <= 16; ++i)
	{
		TESTsurface_row_pitch(i, 1);
		TESTsurface_row_pitch(1, i);
		TESTsurface_depth_pitch(i, 1);
		TESTsurface_depth_pitch(1, i);
		TESTsurface_slice_pitch(i);
	}

	surface::info inf;
	inf.dimensions = int3(512,512,511);
	inf.array_size = 1;
	inf.format = surface::r8g8b8a8_unorm;
	inf.layout = surface::tight;
	oCHECK(surface::offset(inf, 1)==0x1ff00000, "");

	int3 MipDimensions[] = 
	{ 
		int3(1,1,1), 
		int3(2,2,2), int3(1,2,2), int3(2,1,2), int3(2,2,1), int3(1,1,2), int3(1,2,1), int3(2,1,1), 
		int3(3,3,3), int3(1,3,3), int3(3,1,3), int3(3,3,1), int3(1,1,3), int3(1,3,1), int3(3,1,1), 
		int3(4,4,4),
		int3(16,16,16),
		int3(1024,1024,1024), int3(1,1024,1024), int3(1024,1,1024), int3(1024,1024,1), int3(1,1,1024), int3(1,1024,1), int3(1,1,1024),
	};
	int NumMips[] = 
	{ 
		1, 
		2, 2, 2, 2, 2, 2, 2, 
		2, 2, 2, 2, 2, 2, 2, 
		3,
		5,
		11, 11, 11, 11, 11, 11, 11
	};

	oFORI(i, MipDimensions)
		oCHECK(surface::num_mips(surface::tight, MipDimensions[i]) == NumMips[i], "surface::num_mips(.., int3(%d,%d,%d))==%d failed", MipDimensions[i].x, MipDimensions[i].y, MipDimensions[i].z, NumMips[i]);
}

	} // namespace tests
} // namespace ouro