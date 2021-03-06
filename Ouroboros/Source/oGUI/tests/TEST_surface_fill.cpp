// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oSurface/fill.h>
#include <oGUI/Windows/win_gdi.h>
#include <oGUI/Windows/win_gdi_bitmap.h>
#include <oGUI/Windows/win_gdi_draw.h>

using namespace ouro::windows::gdi;

namespace ouro
{

void core_fill_grid_numbers(surface::image* _pBuffer, const int2& _GridDimensions, color _NumberColor)
{
	surface::info si = _pBuffer->get_info();

	if (si.format != surface::format::b8g8r8a8_unorm)
		throw std::invalid_argument("only b8g8r8a8_unorm currently supported");

	scoped_bitmap hBmp = make_bitmap(_pBuffer);
	scoped_getdc hScreenDC(nullptr);
	scoped_compat hDC(hScreenDC);

	font_info fd;
	fd.name = "Tahoma";
	fd.point_size = (logical_height_to_pointf(hDC, _GridDimensions.y) * 0.33f);
	fd.bold = fd.point_size < 15;
	scoped_font hFont(make_font(fd));

	scoped_select ScopedSelectBmp(hDC, hBmp);
	scoped_select ScopedSelectFont(hDC, hFont);

	text_info td;
	td.alignment = alignment::middle_center;
	td.shadow = color(0);
	td.single_line = true;

	surface::fill_grid_numbers(si.dimensions.xy(), _GridDimensions,
		[&](const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)->bool
	{
		td.position = _DrawBoxPosition;
		td.size = _DrawBoxSize;
		td.foreground = _NumberColor;
		draw_text(hDC, td, _Text);
		return true;
	});

	surface::lock_guard lock(_pBuffer);
	windows::gdi::memcpy2d(lock.mapped.data, lock.mapped.row_pitch, hBmp, si.dimensions.y, true);
}

}
