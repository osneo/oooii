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
#include <oSurface/codec.h>
#include <oBase/allocate.h>
#include <oBase/finally.h>
#include <oBase/throw.h>
#undef JPEG_LIB_VERSION
#define JPEG_LIB_VERSION 80
#include <libjpegTurbo/jpeglib.h>

// this is set at the start of encode and/or decode to pass through to memory hooks 
static oTHREAD_LOCAL const ouro::allocator* tl_alloc;

namespace ouro { namespace surface {

void error_exit_throw(j_common_ptr cinfo)
{
  char buffer[JMSG_LENGTH_MAX];
  (*cinfo->err->format_message)(cinfo, buffer);
	throw std::exception(buffer);
}

void dont_output_message(j_common_ptr cinfo)
{
}

// @tony: Why'd we do this again? and not to other libraries? Was this subsumed 
// by the custom turbo-jpeg effort at OOOii? Can I get rid of all this?

void* o_get_small(j_common_ptr cinfo, size_t sizeofobject)
{
	return tl_alloc->allocate(sizeofobject, 0);
}

void o_free_small(j_common_ptr cinfo, void* object, size_t sizeofobject)
{
	tl_alloc->deallocate(object);
}

void* o_get_large(j_common_ptr cinfo, size_t sizeofobject)
{
	return tl_alloc->allocate(sizeofobject, 0);
}

void o_free_large(j_common_ptr cinfo, void FAR* object, size_t sizeofobject)
{
	tl_alloc->deallocate(object);
}

size_t o_mem_available(j_common_ptr cinfo, size_t min_bytes_needed, size_t max_bytes_needed, size_t already_allocated)
{
	return max_bytes_needed;
}

long o_init(j_common_ptr cinfo)
{
	return 0;
}

void o_term(j_common_ptr cinfo)
{
}

static ouro_jpeg_memory_alloc s_jalloc =
{
	o_get_small,
	o_free_small,
	o_get_large,
	o_free_large,
	o_mem_available,
	o_init,
	o_term
};

static J_COLOR_SPACE to_jcs(format _Format, int* _NumComponents)
{
	switch (_Format)
	{
		case format::r8_unorm: *_NumComponents = 1; return JCS_GRAYSCALE;
		case format::r8g8b8_unorm: *_NumComponents = 3; return JCS_RGB;
		case format::b8g8r8_unorm: *_NumComponents = 3; return JCS_EXT_BGR;
		case format::r8g8b8a8_unorm: *_NumComponents = 4; return JCS_EXT_RGBA;
		case format::b8g8r8a8_unorm: *_NumComponents = 4; return JCS_EXT_BGRA;
		default: break;
	}
	return JCS_UNKNOWN;
}

static format from_jcs(J_COLOR_SPACE _ColorSpace)
{
	switch (_ColorSpace)
	{
		case JCS_GRAYSCALE: return format::r8_unorm;
		case JCS_RGB: return format::r8g8b8_unorm;
		case JCS_YCbCr: return format::y8_u8v8_unorm;
		//case JCS_CMYK: return ?;/* C/M/Y/K */
		//case JCS_YCCK: return ?;/* Y/Cb/Cr/K */
		//case JCS_RGBX: return ?;
		case JCS_EXT_BGR: return format::b8g8r8_unorm;
		case JCS_EXT_BGRX: return format::b8g8r8x8_unorm;
		case JCS_EXT_XBGR: return format::x8b8g8r8_unorm;
		//case JCS_EXT_XRGB: return ?;
		case JCS_EXT_RGBA: return format::r8g8b8a8_unorm;
		case JCS_EXT_BGRA: return format::b8g8r8a8_unorm;
		case JCS_EXT_ABGR: return format::a8b8g8r8_unorm;
		//case JCS_EXT_ARGB: return ?;
		default: break;
	}
	return format::unknown;
}

bool is_jpg(const void* _pBuffer, size_t _BufferSize)
{
	static const uint8_t jpg_sig1[4] = { 0xff, 0xd8, 0xff, 0xe0 };
	static const char jpg_sig2[5] = "JFIF";

	return _BufferSize >= 11 && !memcmp(jpg_sig1, _pBuffer, sizeof(jpg_sig1))
		&& !memcmp(jpg_sig2, ((const uint8_t*)_pBuffer) + 6, sizeof(jpg_sig2));
}

format required_input_jpg(const format& stored)
{
	if (num_channels(stored) == 4)
		return format::b8g8r8a8_unorm;
	else if (num_channels(stored) == 3)
		return format::b8g8r8_unorm;
	return format::unknown;
}

info get_info_jpg(const void* _pBuffer, size_t _BufferSize)
{
	if (!is_jpg(_pBuffer, _BufferSize))
		return info();

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_decompress(&cinfo);
	finally Destroy([&] { jpeg_destroy_decompress(&cinfo); });
	jpeg_mem_src(&cinfo, (uchar*)_pBuffer, static_cast<unsigned long>(_BufferSize));
	jpeg_read_header(&cinfo, TRUE);

	info si;
	si.format = from_jcs(cinfo.jpeg_color_space);
	si.mip_layout = mip_layout::none;
	si.dimensions = int3(cinfo.image_width, cinfo.image_height, 1);
	return si;
}

scoped_allocation encode_jpg(const image& img, const allocator& file_alloc, const allocator& temp_alloc, const compression& compression)
{
	tl_alloc = &file_alloc;
	finally reset_alloc([&] { tl_alloc = nullptr; });

	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_compress(&cinfo);

	uchar* pCompressed = nullptr;
	unsigned long CompressedSize = 0;
	finally Destroy([&] { jpeg_destroy_compress(&cinfo); if (pCompressed) tl_alloc->deallocate(pCompressed); });

	info si = img.get_info();
	cinfo.image_width = si.dimensions.x;
	cinfo.image_height = si.dimensions.y;
	cinfo.in_color_space = to_jcs(si.format, &cinfo.input_components);
	
	jpeg_set_defaults(&cinfo);

	int quality = 100;
	switch (compression)
	{
		case compression::none: quality = 100; break;
		case compression::low: quality = 95; break;
		case compression::medium: quality = 75; break;
		case compression::high: quality = 50; break;
		default: throw std::exception("invalid compression");
	}

	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_mem_dest(&cinfo, &pCompressed, &CompressedSize);
	jpeg_start_compress(&cinfo, true);

	{
		JSAMPROW row[1];
		shared_lock lock(img);
		while (cinfo.next_scanline < cinfo.image_height)
		{
			row[0] = (JSAMPLE*)byte_add(lock.mapped.data, cinfo.next_scanline * lock.mapped.row_pitch);
			jpeg_write_scanlines(&cinfo, row, 1);
		}
	}

	jpeg_finish_compress(&cinfo);
	scoped_allocation alloc(pCompressed, CompressedSize, tl_alloc->deallocate);
	pCompressed = nullptr; // so the finally doesn't kill it

	return alloc;
}

image decode_jpg(const void* buffer, size_t size, const allocator& texel_alloc, const allocator& temp_alloc, const mip_layout& layout)
{
	tl_alloc = &temp_alloc;
	//finally reset_alloc([&] { tl_alloc = nullptr; });

	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jerr.error_exit = error_exit_throw;
	jerr.output_message = dont_output_message;
	cinfo.alloc = &s_jalloc;

	jpeg_create_decompress(&cinfo);
	finally Destroy([&] { jpeg_destroy_decompress(&cinfo); });
	jpeg_mem_src(&cinfo, (uchar*)buffer, static_cast<ulong>(size));
	jpeg_read_header(&cinfo, TRUE);

	info si;
	si.format = from_jcs(cinfo.out_color_space);
	si.mip_layout = layout;
	si.dimensions = int3(cinfo.image_width, cinfo.image_height, 1);

	image img(si, texel_alloc);
	{
		JSAMPROW row[1];
		lock_guard lock(img);
		jpeg_start_decompress(&cinfo);

		while (cinfo.output_scanline < cinfo.output_height)
		{
			row[0] = (JSAMPLE*)byte_add(lock.mapped.data, cinfo.output_scanline * lock.mapped.row_pitch);
			jpeg_read_scanlines(&cinfo, row, 1);
		}
	}

	jpeg_finish_decompress(&cinfo);
	return img;
}

}}
