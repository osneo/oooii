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
#include <oBasis/oCameraController.h>

const oGUID& oGetGUID(threadsafe const oCameraController* threadsafe const*)
{
	// {D7874299-6F52-4E5F-A76E-9ACB37A35316}
	static const oGUID IIDCameraController = { 0xd7874299, 0x6f52, 0x4e5f, { 0xa7, 0x6e, 0x9a, 0xcb, 0x37, 0xa3, 0x53, 0x16 } };
	return IIDCameraController;
}

static const char* oAsStringCCFlags(unsigned int _CCFlags)
{
	switch (_CCFlags)
	{
		case oCAMERA_CONTROLLER_ROTATING_YAW: return "oCAMERA_CONTROLLER_ROTATING_YAW";
		case oCAMERA_CONTROLLER_ROTATING_PITCH: return "oCAMERA_CONTROLLER_ROTATING_PITCH";
		case oCAMERA_CONTROLLER_ROTATING_ROLL: return "oCAMERA_CONTROLLER_ROTATING_ROLL";
		case oCAMERA_CONTROLLER_ROTATING_AROUND_COI: return "oCAMERA_CONTROLLER_ROTATING_AROUND_COI";
		case oCAMERA_CONTROLLER_TRANSLATING_X: return "oCAMERA_CONTROLLER_TRANSLATING_X";
		case oCAMERA_CONTROLLER_TRANSLATING_Y: return "oCAMERA_CONTROLLER_TRANSLATING_Y";
		case oCAMERA_CONTROLLER_TRANSLATING_Z: return "oCAMERA_CONTROLLER_TRANSLATING_Z";
		case oCAMERA_CONTROLLER_SHOW_POINTER: return "oCAMERA_CONTROLLER_SHOW_POINTER";
		case oCAMERA_CONTROLLER_HIDE_POINTER: return "oCAMERA_CONTROLLER_HIDE_POINTER";
		case oCAMERA_CONTROLLER_LOCK_POINTER: return "oCAMERA_CONTROLLER_LOCK_POINTER";
		case oCAMERA_CONTROLLER_UNLOCK_POINTER: return "oCAMERA_CONTROLLER_UNLOCK_POINTER";
		case oCAMERA_CONTROLLER_SAVE_POINTER_POSITION: return "oCAMERA_CONTROLLER_SAVE_POINTER_POSITION";
		case oCAMERA_CONTROLLER_LOAD_POINTER_POSITION: return "oCAMERA_CONTROLLER_LOAD_POINTER_POSITION";
		oNODEFAULT;
	}
}

char* oCameraControllerParseResponse(char* _StrDestination, size_t _SizeofStrDestination, int _ResponseFlags)
{
	return oAsStringFlags(_StrDestination, _SizeofStrDestination, (unsigned int)_ResponseFlags, "0", [&](unsigned int _Flag) { return oAsStringCCFlags(_Flag); });
}