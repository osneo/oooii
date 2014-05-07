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
#include <oPlatform/oHTTPHandler.h>
#include <oPlatform/oStream.h>
#include <oBasis/oRTTI.h>

using namespace ouro;

oRTTI_COMPOUND_BEGIN_DESCRIPTION(oRTTI_CAPS_NONE, oHTTPHandlerCommonQueryParams)
	oRTTI_COMPOUND_ABSTRACT(oHTTPHandlerCommonQueryParams)
	oRTTI_COMPOUND_ATTRIBUTES_BEGIN(oHTTPHandlerCommonQueryParams)
		oRTTI_COMPOUND_ATTR(oHTTPHandlerCommonQueryParams, Range, oRTTI_OF(int2), "range", oRTTI_COMPOUND_ATTR_REGULAR)
		oRTTI_COMPOUND_ATTR(oHTTPHandlerCommonQueryParams, Count, oRTTI_OF(bool), "count", oRTTI_COMPOUND_ATTR_REGULAR)
	oRTTI_COMPOUND_ATTRIBUTES_END(oHTTPHandlerCommonQueryParams)
oRTTI_COMPOUND_END_DESCRIPTION(oHTTPHandlerCommonQueryParams)

void oHTTPHandlerBuildStaticFileResponse(const char* _FullPath, oHTTPHandler::CommonParams& _CommonParams)
{
	try
	{
		scoped_allocation alloc = filesystem::load(path(_FullPath));
		_CommonParams.AllocateResponse(alloc.size());
		memcpy(_CommonParams.pResponse->Content.pData, alloc, alloc.size());
	}
	catch (std::exception&)
	{
		return;
	}
	oMIMEFromExtension(&_CommonParams.pResponse->Content.Type, path(_FullPath).extension());
	_CommonParams.pResponse->StatusLine.StatusCode = oHTTP_OK;
}

void oHTTPHandlerErrorV(oHTTPHandler::CommonParams& _CommonParams, oHTTP_STATUS_CODE _HTTPErrorCode, const char* _Format, va_list _Args)
{ 
	xxlstring buildString;
	vsnprintf(buildString, _Format, _Args);

	_CommonParams.AllocateResponse(buildString.length());
	memcpy(_CommonParams.pResponse->Content.pData, buildString.c_str(), buildString.length());
	_CommonParams.pResponse->Content.Type = oMIME_TEXT_PLAIN;
	_CommonParams.pResponse->StatusLine.StatusCode = _HTTPErrorCode;
}

void oHTTPHandlerError(oHTTPHandler::CommonParams& _CommonParams, oHTTP_STATUS_CODE _HTTPErrorCode, const char* _Format, ...)
{ 
	va_list args;
	va_start(args, _Format);
	oHTTPHandlerErrorV(_CommonParams, _HTTPErrorCode, _Format, args);
	va_end(args);
}