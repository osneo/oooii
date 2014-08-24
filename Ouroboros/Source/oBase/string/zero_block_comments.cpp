/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <memory.h>
#include <oBase/macros.h>

namespace ouro {

char* next_matching(char* _pPointingAtOpenBrace, const char* _OpenBrace, const char* _CloseBrace);

char* zero_block_comments(char* _String, const char* _OpenBrace, const char* _CloseBrace, char _Replacement)
{
	char* cur = _String;
	while (*cur)
	{
		cur = strstr(_String, _OpenBrace);
		if (!cur)
			break;

		char* close = next_matching(cur, _OpenBrace, _CloseBrace);
		if (!close)
			return nullptr;

		close += strlen(_CloseBrace);

		// preserve newlines since they might be used to report line numbers in compile errors
		while (cur < close)
		{
			size_t offset = __min(strcspn(cur, oNEWLINE), static_cast<size_t>(close-cur));
			memset(cur, _Replacement, offset);
			cur += offset;
			cur += strspn(cur, oNEWLINE);
		}
	}

	return _String;
}

}
