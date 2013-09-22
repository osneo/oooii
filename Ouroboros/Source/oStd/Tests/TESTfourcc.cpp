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
#include <oStd/fourcc.h>
#include <oStd/stringize.h>
#include <oStd/throw.h>

namespace oStd {
	namespace tests {

void TESTfourcc()
{
	fourcc fcc('TEST');

	char str[16];

	if (!to_string(str, fcc))
		oTHROW(protocol_error, "to_string on fourcc failed 1");

	if (strcmp("TEST", str))
		oTHROW(protocol_error, "to_string on fourcc failed 2");

	const char* fccStr = "RGBA";
	if (!from_string(&fcc, fccStr))
		oTHROW(protocol_error, "from_string on fourcc failed 1");

	if (fourcc('RGBA') != fcc)
		oTHROW(protocol_error, "from_string on fourcc failed 2");
}

	} // namespace tests
} // namespace oStd