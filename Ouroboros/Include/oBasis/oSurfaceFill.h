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
// Simple utilities for filling a 2D surface with color in various patterns. 
// This is useful in generating debug images/textures for infrastructure 
// bringup.
#ifndef oSurfaceFill_h
#define oSurfaceFill_h

#include <oBasis/oColor.h>
#include <oBasis/oFunction.h>
#include <oBasis/oMathTypes.h>

// Fills the specified 32-bit BGRA (oColor) buffer with the specified solid 
// color.
void oSurfaceFillSolid(oColor* _pColors, size_t _RowPitch, const int2& _Dimensions, const oColor& _Color);

// Fills the specified 32-bit BGRA (oColor buffer) with a checkerboard pattern
// of the specified dimensions and two colors.
void oSurfaceFillCheckerboard(oColor* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const oColor& _Color0, const oColor& _Color1);

// Fills the specified 32-bit BGRA buffer with a gradient that goes between the
// 4 specified colors at the corners of the image.
void oSurfaceFillGradient(oColor* _pColors, size_t _RowPitch, const int2& _Dimensions, oColor _CornerColors[4]);

// This draws a rectangle at [(0,0),_GridDim-int2(1,1)] in the specified color,
// thus producing a grid pattern.
void oSurfaceFillGridLines(oColor* _pColors, size_t _RowPitch, const int2& _Dimensions, const int2& _GridDimensions, const oColor& _GridColor);

// This iterates through each grid box and calls the _DrawText function
bool oSurfaceFillGridNumbers(const int2& _Dimensions, const int2& _GridDimensions, oFUNCTION<bool(const int2& _DrawBoxPosition, const int2& _DrawBoxSize, const char* _Text)> _DrawText);

#endif