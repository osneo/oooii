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
#include <oGUI/Windows/win_as_string.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/windows/win_error.h>

#undef interface
#include <Windowsx.h>
#include <DShow.h>
#include <Shellapi.h>
#include <CDErr.h>
#include <Dbt.h>
#include <SetupAPI.h>

namespace ouro {
	namespace windows {
		namespace as_string {

const char* HT(int _HTCode)
{
	switch (_HTCode)
	{
		case HTERROR: return "HTERROR";
		case HTTRANSPARENT: return "HTTRANSPARENT";
		case HTNOWHERE: return "HTNOWHERE";
		case HTCLIENT: return "HTCLIENT";
		case HTCAPTION: return "HTCAPTION";
		case HTSYSMENU: return "HTSYSMENU";
		//case HTGROWBOX: return "HTGROWBOX"; // same as HTSIZE
		case HTSIZE: return "HTSIZE";
		case HTMENU: return "HTMENU";
		case HTHSCROLL: return "HTHSCROLL";
		case HTVSCROLL: return "HTVSCROLL";
		case HTMINBUTTON: return "HTMINBUTTON";
		case HTMAXBUTTON: return "HTMAXBUTTON";
		case HTLEFT: return "HTLEFT";
		case HTRIGHT: return "HTRIGHT";
		case HTTOP: return "HTTOP";
		case HTTOPLEFT: return "HTTOPLEFT";
		case HTTOPRIGHT: return "HTTOPRIGHT";
		case HTBOTTOM: return "HTBOTTOM";
		case HTBOTTOMLEFT: return "HTBOTTOMLEFT";
		case HTBOTTOMRIGHT: return "HTBOTTOMRIGHT";
		case HTBORDER: return "HTBORDER";
		//case HTREDUCE: return "HTREDUCE"; // same as HTMINBUTTON
		//case HTZOOM: return "HTZOOM"; // same as HTMAXBUTTON
		//case HTSIZEFIRST: return "HTSIZEFIRST"; // same as HTLEFT
		//case HTSIZELAST: return "HTSIZELAST"; // same as HTBOTTOMRIGHT
		#if(WINVER >= 0x0400)
			case HTOBJECT: return "HTOBJECT";
			case HTCLOSE: return "HTCLOSE";
			case HTHELP: return "HTHELP";
		#endif
		default: break;
	}
	return "unrecognized HTCODE";
}

const char* SC(int _SCCode)
{
#define oSC_DRAGMOVE 0xf012

	switch (_SCCode)
	{
		case SC_SIZE: return "SC_SIZE";
		case SC_MOVE: return "SC_MOVE";
		case SC_MINIMIZE: return "SC_MINIMIZE";
		case SC_MAXIMIZE: return "SC_MAXIMIZE";
		case SC_NEXTWINDOW: return "SC_NEXTWINDOW";
		case SC_PREVWINDOW: return "SC_PREVWINDOW";
		case SC_CLOSE: return "SC_CLOSE";
		case SC_VSCROLL: return "SC_VSCROLL";
		case SC_HSCROLL: return "SC_HSCROLL";
		case SC_MOUSEMENU: return "SC_MOUSEMENU";
		case SC_KEYMENU: return "SC_KEYMENU";
		case SC_ARRANGE: return "SC_ARRANGE";
		case SC_RESTORE: return "SC_RESTORE";
		case SC_TASKLIST: return "SC_TASKLIST";
		case SC_SCREENSAVE: return "SC_SCREENSAVE";
		case SC_HOTKEY: return "SC_HOTKEY";
		#if(WINVER >= 0x0400)
			case SC_DEFAULT: return "SC_DEFAULT";
			case SC_MONITORPOWER: return "SC_MONITORPOWER";
			case SC_CONTEXTHELP: return "SC_CONTEXTHELP";
			case SC_SEPARATOR: return "SC_SEPARATOR";
		#endif
		case oSC_DRAGMOVE: return "SC_DRAGMOVE";
		default: break;
	}

	return "unrecognized SCCODE";
}

const char* SW(int _SWCode)
{
	switch (_SWCode)
	{
		case SW_OTHERUNZOOM: return "SW_OTHERUNZOOM";
		case SW_OTHERZOOM: return "SW_OTHERZOOM";
		case SW_PARENTCLOSING: return "SW_PARENTCLOSING";
		case SW_PARENTOPENING: return "SW_PARENTOPENING";
		default: break;
	}
	return "unrecognized SWCODE";
}

const char* WM(int _uMsg)
{
	switch (_uMsg)
	{
		case WM_ACTIVATE: return "WM_ACTIVATE";
		case WM_ACTIVATEAPP: return "WM_ACTIVATEAPP";
		case WM_APPCOMMAND: return "WM_APPCOMMAND";
		case WM_CANCELMODE: return "WM_CANCELMODE";
		case WM_CAPTURECHANGED: return "WM_CAPTURECHANGED";
		case WM_CHAR: return "WM_CHAR";
		case WM_CHILDACTIVATE: return "WM_CHILDACTIVATE";
		case WM_CLOSE: return "WM_CLOSE";
		case WM_COMMAND: return "WM_COMMAND";
		case WM_COMPACTING: return "WM_COMPACTING";
		case WM_COMPAREITEM: return "WM_COMPAREITEM";
		case WM_CREATE: return "WM_CREATE";
		case WM_DEADCHAR: return "WM_DEADCHAR";
		case WM_DESTROY: return "WM_DESTROY";
		case WM_DISPLAYCHANGE: return "WM_DISPLAYCHANGE";
		case WM_DEVICECHANGE: return "WM_DEVICECHANGE";
		case WM_DRAWITEM: return "WM_DRAWITEM";
		case WM_DROPFILES: return "WM_DROPFILES";
		case WM_DWMCOLORIZATIONCOLORCHANGED: return "WM_DWMCOLORIZATIONCOLORCHANGED";
		case WM_DWMCOMPOSITIONCHANGED: return "WM_DWMCOMPOSITIONCHANGED";
		case WM_DWMNCRENDERINGCHANGED: return "WM_DWMNCRENDERINGCHANGED";
		case WM_DWMWINDOWMAXIMIZEDCHANGE: return "WM_DWMWINDOWMAXIMIZEDCHANGE";
		case WM_ENABLE: return "WM_ENABLE";
		case WM_ENTERIDLE: return "WM_ENTERIDLE";
		case WM_ENTERSIZEMOVE: return "WM_ENTERSIZEMOVE";
		case WM_ERASEBKGND: return "WM_ERASEBKGND";
		case WM_EXITSIZEMOVE: return "WM_EXITSIZEMOVE";
		case WM_GETDLGCODE: return "WM_GETDLGCODE";
		case WM_GETICON: return "WM_GETICON";
		case WM_GETMINMAXINFO: return "WM_GETMINMAXINFO";
		case WM_GETTEXT: return "WM_GETTEXT";
		case WM_GETTEXTLENGTH: return "WM_GETTEXTLENGTH";
		case WM_HELP: return "WM_HELP";
		case WM_HOTKEY: return "WM_HOTKEY";
		case WM_ICONERASEBKGND: return "WM_ICONERASEBKGND";
		case WM_INITMENUPOPUP: return "WM_INITMENUPOPUP";
		case WM_INPUT: return "WM_INPUT";
		case WM_INPUTLANGCHANGE: return "WM_INPUTLANGCHANGE";
		case WM_INPUTLANGCHANGEREQUEST: return "WM_INPUTLANGCHANGEREQUEST";
		case WM_KEYDOWN: return "WM_KEYDOWN";
		case WM_KEYUP: return "WM_KEYUP";
		case WM_KILLFOCUS: return "WM_KILLFOCUS";
		case WM_MEASUREITEM: return "WM_MEASUREITEM";
		case WM_MENUSELECT: return "WM_MENUSELECT";
		case WM_MOVE: return "WM_MOVE";
		case WM_MOVING: return "WM_MOVING";
		case WM_NCMOUSEMOVE: return "WM_NCMOUSEMOVE";
		case WM_NCMOUSELEAVE: return "WM_NCMOUSELEAVE";
		case WM_NCACTIVATE: return "WM_NCACTIVATE";
		case WM_NCCALCSIZE: return "WM_NCCALCSIZE";
		case WM_NCCREATE: return "WM_NCCREATE";
		case WM_NCDESTROY: return "WM_NCDESTROY";
		case WM_NCPAINT: return "WM_NCPAINT";
		case WM_NCLBUTTONDOWN: return "WM_NCLBUTTONDOWN";
		case WM_NOTIFY: return "WM_NOTIFY";
		case WM_NOTIFYFORMAT: return "WM_NOTIFYFORMAT";
		case WM_NULL: return "WM_NULL";
		case WM_PAINT: return "WM_PAINT";
		case WM_PRINT: return "WM_PRINT";
		case WM_PRINTCLIENT: return "WM_PRINTCLIENT";
		case WM_PAINTICON: return "WM_PAINTICON";
		case WM_PARENTNOTIFY: return "WM_PARENTNOTIFY";
		case WM_QUERYDRAGICON: return "WM_QUERYDRAGICON";
		case WM_QUERYOPEN: return "WM_QUERYOPEN";
		case WM_NCHITTEST: return "WM_NCHITTEST";
		case WM_SETCURSOR: return "WM_SETCURSOR";
		case WM_SETFOCUS: return "WM_SETFOCUS";
		case WM_SETICON: return "WM_SETICON";
		case WM_SETREDRAW: return "WM_SETREDRAW";
		case WM_SETTEXT: return "WM_SETTEXT";
		case WM_SHOWWINDOW: return "WM_SHOWWINDOW";
		case WM_SIZE: return "WM_SIZE";
		case WM_SIZING: return "WM_SIZING";
		case WM_STYLECHANGED: return "WM_STYLECHANGED";
		case WM_STYLECHANGING: return "WM_STYLECHANGING";
		case WM_SYNCPAINT: return "WM_SYNCPAINT";
		case WM_SYSDEADCHAR: return "WM_SYSDEADCHAR";
		case WM_SYSKEYDOWN: return "WM_SYSKEYDOWN";
		case WM_SYSKEYUP: return "WM_SYSKEYUP";
		case WM_SYSCOMMAND: return "WM_SYSCOMMAND";
		case WM_THEMECHANGED: return "WM_THEMECHANGED";
		case WM_UNICHAR: return "WM_UNICHAR";
		case WM_USERCHANGED: return "WM_USERCHANGED";
		case WM_WINDOWPOSCHANGED: return "WM_WINDOWPOSCHANGED";
		case WM_WINDOWPOSCHANGING: return "WM_WINDOWPOSCHANGING";
		case WM_MOUSEACTIVATE: return "WM_MOUSEACTIVATE";
		case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
		case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
		case WM_LBUTTONUP: return "WM_LBUTTONUP";
		case WM_MBUTTONDOWN: return "WM_MBUTTONDOWN";
		case WM_MBUTTONUP: return "WM_MBUTTONUP";
		case WM_RBUTTONDOWN: return "WM_RBUTTONDOWN";
		case WM_RBUTTONUP: return "WM_RBUTTONUP";
		case WM_XBUTTONDOWN: return "WM_XBUTTONDOWN";
		case WM_XBUTTONUP: return "WM_XBUTTONUP";
		case WM_MOUSEWHEEL: return "WM_MOUSEWHEEL";
		case WM_MOUSEHWHEEL: return "WM_MOUSEHWHEEL";
		case WM_CONTEXTMENU: return "WM_CONTEXTMENU";
		case WM_IME_SETCONTEXT: return "WM_IME_SETCONTEXT";
		case WM_IME_NOTIFY: return "WM_IME_NOTIFY";
		case WM_INITMENU: return "WM_INITMENU";
		case WM_ENTERMENULOOP: return "WM_ENTERMENULOOP";
		case WM_EXITMENULOOP: return "WM_EXITMENULOOP";
		case WM_INITDIALOG: return "WM_INITDIALOG";
		case WM_CTLCOLORBTN: return "WM_CTLCOLORBTN";
		case WM_CTLCOLORDLG: return "WM_CTLCOLORDLG";
		case WM_CTLCOLOREDIT: return "WM_CTLCOLOREDIT";
		case WM_CTLCOLORLISTBOX: return "WM_CTLCOLORLISTBOX";
		case WM_CTLCOLORMSGBOX: return "WM_CTLCOLORMSGBOX";
		case WM_CTLCOLORSCROLLBAR: return "WM_CTLCOLORSCROLLBAR";
		case WM_CTLCOLORSTATIC: return "WM_CTLCOLORSTATIC";
		case WM_UPDATEUISTATE: return "WM_UPDATEUISTATE";
		case WM_CHANGEUISTATE: return "WM_CHANGEUISTATE";
		case WM_QUERYUISTATE: return "WM_QUERYUISTATE";
		case 0x91: return "Theme Service internal message 0x91";
		case 0x92: return "Theme Service internal message 0x92";
		case 0x93: return "Theme Service internal message 0x93";

		// Ouroboros-custom messages
		case oWM_DISPATCH: return "oWM_DISPATCH";
		case oWM_DESTROY: return "oWM_DESTROY";
		case oWM_SKELETON: return "oWM_SKELETON";
		case oWM_USER_CAPTURED: return "oWM_USER_CAPTURED";
		case oWM_USER_LOST: return "oWM_USER_LOST";
		case oWM_INPUT_DEVICE_CHANGE: return "oWM_INPUT_DEVICE_CHANGE";

		default: break;
	}
	return "unrecognized WMCODE";
}

const char* WS(int _WSFlag)
{
	switch (_WSFlag)
	{
		case WS_OVERLAPPED: return "WS_OVERLAPPED";
		case WS_POPUP: return "WS_POPUP";
		case WS_CHILD: return "WS_CHILD";
		case WS_MINIMIZE: return "WS_MINIMIZE";
		case WS_VISIBLE: return "WS_VISIBLE";
		case WS_DISABLED: return "WS_DISABLED";
		case WS_CLIPSIBLINGS: return "WS_CLIPSIBLINGS";
		case WS_CLIPCHILDREN: return "WS_CLIPCHILDREN";
		case WS_MAXIMIZE: return "WS_MAXIMIZE";
		case WS_CAPTION: return "WS_CAPTION";
		case WS_BORDER: return "WS_BORDER";
		case WS_DLGFRAME: return "WS_DLGFRAME";
		case WS_VSCROLL: return "WS_VSCROLL";
		case WS_HSCROLL: return "WS_HSCROLL";
		case WS_SYSMENU: return "WS_SYSMENU";
		case WS_THICKFRAME: return "WS_THICKFRAME";
		case WS_GROUP: return "WS_GROUP";
		case WS_TABSTOP: return "WS_TABSTOP";
		//case WS_MINIMIZEBOX: return "WS_MINIMIZEBOX"; // already used
		//case WS_MAXIMIZEBOX: return "WS_MAXIMIZEBOX"; // already used
		default: break;
	}

	return "unrecognized WS flag";
}

const char* WSEX(int _WSEXFlag)
{
	switch (_WSEXFlag)
	{
		case WS_EX_DLGMODALFRAME: return "WS_EX_DLGMODALFRAME";
		case WS_EX_NOPARENTNOTIFY: return "WS_EX_NOPARENTNOTIFY";
		case WS_EX_TOPMOST: return "WS_EX_TOPMOST";
		case WS_EX_ACCEPTFILES: return "WS_EX_ACCEPTFILES";
		case WS_EX_TRANSPARENT: return "WS_EX_TRANSPARENT";
		case WS_EX_MDICHILD: return "WS_EX_MDICHILD";
		case WS_EX_TOOLWINDOW: return "WS_EX_TOOLWINDOW";
		case WS_EX_WINDOWEDGE: return "WS_EX_WINDOWEDGE";
		case WS_EX_CLIENTEDGE: return "WS_EX_CLIENTEDGE";
		case WS_EX_CONTEXTHELP: return "WS_EX_CONTEXTHELP";
		case WS_EX_RIGHT: return "WS_EX_RIGHT";
		case WS_EX_LEFT: return "WS_EX_LEFT";
		case WS_EX_RTLREADING: return "WS_EX_RTLREADING";
		//case WS_EX_LTRREADING: return "WS_EX_LTRREADING"; // dup b/c val is 0
		case WS_EX_LEFTSCROLLBAR: return "WS_EX_LEFTSCROLLBAR";
		//case WS_EX_RIGHTSCROLLBAR: return "WS_EX_RIGHTSCROLLBAR"; // dup b/c val is 0
		case WS_EX_CONTROLPARENT: return "WS_EX_CONTROLPARENT";
		case WS_EX_STATICEDGE: return "WS_EX_STATICEDGE";
		case WS_EX_APPWINDOW: return "WS_EX_APPWINDOW";
		case WS_EX_OVERLAPPEDWINDOW: return "WS_EX_OVERLAPPEDWINDOW";
		case WS_EX_PALETTEWINDOW: return "WS_EX_PALETTEWINDOW";
		case WS_EX_LAYERED: return "WS_EX_LAYERED";
		case WS_EX_NOINHERITLAYOUT: return "WS_EX_NOINHERITLAYOUT";
		case WS_EX_LAYOUTRTL: return "WS_EX_LAYOUTRTL";
		case WS_EX_COMPOSITED: return "WS_EX_COMPOSITED";
		case WS_EX_NOACTIVATE: return "WS_EX_NOACTIVATE";
		default: break;
	}
	
	return "unrecognized WS_EX flag";
}

const char* WA(int _WACode)
{
	switch (_WACode)
	{
		case WA_ACTIVE: return "WA_ACTIVE";
		case WA_CLICKACTIVE: return "WA_CLICKACTIVE";
		case WA_INACTIVE: return "WA_INACTIVE";
		default: break;
	}

	return "unrecognized WACode";
}

const char* BST(int _BSTCode)
{
	switch (_BSTCode)
	{
		case BST_CHECKED: return "BST_CHECKED";
		case BST_DROPDOWNPUSHED: return "BST_DROPDOWNPUSHED";
		case BST_FOCUS: return "BST_FOCUS";
		case BST_HOT: return "BST_HOT";
		case BST_INDETERMINATE: return "BST_INDETERMINATE";
		case BST_PUSHED: return "BST_PUSHED";
		case BST_UNCHECKED: return "BST_UNCHECKED";
		default: break;
	}

	return "unrecognized BSTCode";
}

const char* NM(int _NMCode)
{
	switch (_NMCode)
	{
		case NM_CHAR: return "NM_CHAR";
		case NM_CUSTOMDRAW: return "NM_CUSTOMDRAW";
		case NM_CUSTOMTEXT: return "NM_CUSTOMTEXT";
		case NM_FONTCHANGED: return "NM_FONTCHANGED";
		case NM_GETCUSTOMSPLITRECT: return "NM_GETCUSTOMSPLITRECT";
		case NM_HOVER: return "NM_HOVER";
		case NM_KEYDOWN: return "NM_KEYDOWN";
		case NM_KILLFOCUS: return "NM_KILLFOCUS";
		case NM_LDOWN: return "NM_LDOWN";
		case NM_NCHITTEST: return "NM_NCHITTEST";
		case NM_OUTOFMEMORY: return "NM_OUTOFMEMORY";
		case NM_RDOWN: return "NM_RDOWN";
		case NM_RELEASEDCAPTURE: return "NM_RELEASEDCAPTURE";
		case NM_RETURN: return "NM_RETURN";
		case NM_SETCURSOR: return "NM_SETCURSOR";
		case NM_SETFOCUS: return "NM_SETFOCUS";
		case NM_THEMECHANGED: return "NM_THEMECHANGED";
		case NM_TOOLTIPSCREATED: return "NM_TOOLTIPSCREATED";
		//case NM_TVSTATEIMAGECHANGING: return "NM_TVSTATEIMAGECHANGING"; // same as NM_CUSTOMTEXT
		case NM_CLICK: return "NM_CLICK";
		case NM_DBLCLK: return "NM_DBLCLK";
		case NM_RCLICK: return "NM_RCLICK";
		case NM_RDBLCLK: return "NM_RDBLCLK";
		default: break;
	}

	return "unrecognized NMCode";
}

const char* SWP(int _SWPCode)
{
	switch (_SWPCode)
	{
		case SWP_NOSIZE: return "SWP_NOSIZE";
		case SWP_NOMOVE: return "SWP_NOMOVE";
		case SWP_NOZORDER: return "SWP_NOZORDER";
		case SWP_NOREDRAW: return "SWP_NOREDRAW";
		case SWP_NOACTIVATE: return "SWP_NOACTIVATE";
		case SWP_FRAMECHANGED: return "SWP_FRAMECHANGED";
		case SWP_SHOWWINDOW: return "SWP_SHOWWINDOW";
		case SWP_HIDEWINDOW: return "SWP_HIDEWINDOW";
		case SWP_NOCOPYBITS: return "SWP_NOCOPYBITS";
		case SWP_NOOWNERZORDER: return "SWP_NOOWNERZORDER";
		case SWP_NOSENDCHANGING: return "SWP_NOSENDCHANGING";
		//case SWP_DRAWFRAME: return "SWP_DRAWFRAME"; // same as DWP_FRAMECHANGED
		//case SWP_NOREPOSITION: return "SWP_NOREPOSITION"; // same as SWP_NOOWNERZORDER
		#if (WINVER >= 0x0400)
			case SWP_DEFERERASE: return "SWP_DEFERERASE";
			case SWP_ASYNCWINDOWPOS: return "SWP_ASYNCWINDOWPOS";
		#endif
		default: break;
	}

	return "unrecognized SWPCode";
}

const char* GWL(int _GWLCode)
{
	switch (_GWLCode)
	{
		#ifndef _WIN64
			case GWL_WNDPROC: return "GWL_WNDPROC";
			case GWL_HINSTANCE: return "GWL_HINSTANCE";
			case GWL_HWNDPARENT: return "GWL_HWNDPARENT";
			case GWL_USERDATA: return "GWL_USERDATA";
		#endif
		case GWL_STYLE: return "GWL_STYLE";
		case GWL_EXSTYLE: return "GWL_EXSTYLE";
		case GWL_ID: return "GWL_ID";
		default: break;
	}

	return "unrecognized GWLCode";
}

const char* GWLP(int _GWLPCode)
{
	switch (_GWLPCode)
	{
		case GWLP_WNDPROC: return "GWLP_WNDPROC";
		case GWLP_HINSTANCE: return "GWLP_HINSTANCE";
		case GWLP_HWNDPARENT: return "GWLP_HWNDPARENT";
		case GWLP_USERDATA: return "GWLP_USERDATA";
		case GWLP_ID: return "GWLP_ID";
		default: break;
	}

	return "unrecognized GWLPCode";
}

const char* TCN(int _TCNCode)
{
	switch (_TCNCode)
	{
		case TCN_KEYDOWN: return "TCN_KEYDOWN";
		case TCN_SELCHANGE: return "TCN_SELCHANGE";
		case TCN_SELCHANGING: return "TCN_SELCHANGING";
		case TCN_GETOBJECT: return "TCN_GETOBJECT";
		case TCN_FOCUSCHANGE: return "TCN_FOCUSCHANGE";
		default: break;
	}

	return "unrecognized TCNCode";
}

const char* DBT(int _DBTEvent)
{
	switch (_DBTEvent)
	{
		case DBT_CONFIGCHANGECANCELED: return "DBT_CONFIGCHANGECANCELED";
		case DBT_CONFIGCHANGED: return "DBT_CONFIGCHANGED";
		case DBT_CUSTOMEVENT: return "DBT_CUSTOMEVENT";
		case DBT_DEVICEARRIVAL: return "DBT_DEVICEARRIVAL";
		case DBT_DEVICEQUERYREMOVE: return "DBT_DEVICEQUERYREMOVE";
		case DBT_DEVICEQUERYREMOVEFAILED: return "DBT_DEVICEQUERYREMOVEFAILED";
		case DBT_DEVICEREMOVECOMPLETE: return "DBT_DEVICEREMOVECOMPLETE";
		case DBT_DEVICEREMOVEPENDING: return "DBT_DEVICEREMOVEPENDING";
		case DBT_DEVICETYPESPECIFIC: return "DBT_DEVICETYPESPECIFIC";
		case DBT_DEVNODES_CHANGED: return "DBT_DEVNODES_CHANGED";
		case DBT_QUERYCHANGECONFIG: return "DBT_QUERYCHANGECONFIG";
		case DBT_USERDEFINED: return "DBT_USERDEFINED";
		default: break;
	}

	return "unrecognized DBT event";
}

const char* DBTDT(int _DBTDevType)
{
	switch (_DBTDevType)
	{
		case DBT_DEVTYP_DEVICEINTERFACE: return "DBT_DEVTYP_DEVICEINTERFACE";
		case DBT_DEVTYP_HANDLE: return "DBT_DEVTYP_HANDLE";
		case DBT_DEVTYP_OEM: return "DBT_DEVTYP_OEM";
		case DBT_DEVTYP_PORT: return "DBT_DEVTYP_PORT";
		case DBT_DEVTYP_VOLUME: return "DBT_DEVTYP_VOLUME";
		default: break;
	}

	return "unrecognized DBT devtype event";
}

const char* SPDRP(int _SPDRPValue)
{
	switch (_SPDRPValue)
	{
		case SPDRP_DEVICEDESC: return "SPDRP_DEVICEDESC";
		case SPDRP_HARDWAREID: return "SPDRP_HARDWAREID";
		case SPDRP_COMPATIBLEIDS: return "SPDRP_COMPATIBLEIDS";
		case SPDRP_UNUSED0: return "SPDRP_UNUSED0";
		case SPDRP_SERVICE: return "SPDRP_SERVICE";
		case SPDRP_UNUSED1: return "SPDRP_UNUSED1";
		case SPDRP_UNUSED2: return "SPDRP_UNUSED2";
		case SPDRP_CLASS: return "SPDRP_CLASS";
		case SPDRP_CLASSGUID: return "SPDRP_CLASSGUID";
		case SPDRP_DRIVER: return "SPDRP_DRIVER";
		case SPDRP_CONFIGFLAGS: return "SPDRP_CONFIGFLAGS";
		case SPDRP_MFG: return "SPDRP_MFG";
		case SPDRP_FRIENDLYNAME: return "SPDRP_FRIENDLYNAME";
		case SPDRP_LOCATION_INFORMATION: return "SPDRP_LOCATION_INFORMATION";
		case SPDRP_PHYSICAL_DEVICE_OBJECT_NAME: return "SPDRP_PHYSICAL_DEVICE_OBJECT_NAME";
		case SPDRP_CAPABILITIES: return "SPDRP_CAPABILITIES";
		case SPDRP_UI_NUMBER: return "SPDRP_UI_NUMBER";
		case SPDRP_UPPERFILTERS: return "SPDRP_UPPERFILTERS";
		case SPDRP_LOWERFILTERS: return "SPDRP_LOWERFILTERS";
		case SPDRP_BUSTYPEGUID: return "SPDRP_BUSTYPEGUID";
		case SPDRP_LEGACYBUSTYPE: return "SPDRP_LEGACYBUSTYPE";
		case SPDRP_BUSNUMBER: return "SPDRP_BUSNUMBER";
		case SPDRP_ENUMERATOR_NAME: return "SPDRP_ENUMERATOR_NAME";
		case SPDRP_SECURITY: return "SPDRP_SECURITY";
		case SPDRP_SECURITY_SDS: return "SPDRP_SECURITY_SDS";
		case SPDRP_DEVTYPE: return "SPDRP_DEVTYPE";
		case SPDRP_EXCLUSIVE: return "SPDRP_EXCLUSIVE";
		case SPDRP_CHARACTERISTICS: return "SPDRP_CHARACTERISTICS";
		case SPDRP_ADDRESS: return "SPDRP_ADDRESS";
		case SPDRP_UI_NUMBER_DESC_FORMAT: return "SPDRP_UI_NUMBER_DESC_FORMAT";
		case SPDRP_DEVICE_POWER_DATA: return "SPDRP_DEVICE_POWER_DATA";
		case SPDRP_REMOVAL_POLICY: return "SPDRP_REMOVAL_POLICY";
		case SPDRP_REMOVAL_POLICY_HW_DEFAULT: return "SPDRP_REMOVAL_POLICY_HW_DEFAULT";
		case SPDRP_REMOVAL_POLICY_OVERRIDE: return "SPDRP_REMOVAL_POLICY_OVERRIDE";
		case SPDRP_INSTALL_STATE: return "SPDRP_INSTALL_STATE";
		case SPDRP_LOCATION_PATHS: return "SPDRP_LOCATION_PATHS";
		case SPDRP_BASE_CONTAINERID: return "SPDRP_BASE_CONTAINERID";
		default: break;
	}
	return "unrecognized SPDRP value";
}

static const char* WMSW(LPARAM _lParam)
{
	switch (_lParam)
	{
		case 0: return "(From ShowWindow call)";
		case SW_OTHERUNZOOM: "SW_OTHERUNZOOM";
		case SW_OTHERZOOM: "SW_OTHERZOOM";
		case SW_PARENTCLOSING: "SW_PARENTCLOSING";
		case SW_PARENTOPENING: "SW_PARENTOPENING";
		default: break;
	}
	return "unrecognized WM_SHOWWINDOW LPARAM value";
}

char* style_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSFlags)
{
	return strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_WSFlags, as_string::WS(WS_OVERLAPPED), as_string::WS);
}

char* style_ex_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _WSEXFlags)
{
	return strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_WSEXFlags, as_string::WSEX(0), as_string::WSEX);
}

char* swp_flags(char* _StrDestination, size_t _SizeofStrDestination, UINT _SWPFlags)
{
	return strbitmask(_StrDestination, _SizeofStrDestination, *(int*)&_SWPFlags & 0x07ff, "0", as_string::SWP);
}

		} // namespace as_string

static char* print_style_change(char* _StrDestination, size_t _SizeofStrDestination, HWND _hWnd, const char* _MsgName, WPARAM _wParam, LPARAM _lParam)
{
	char tmp[1024];
	char tmp2[1024];
	const char* StyleName = "GWL_STYLE";
	if (_wParam == GWL_EXSTYLE)
	{
		as_string::style_ex_flags(tmp, ((STYLESTRUCT*)_lParam)->styleOld);
		as_string::style_ex_flags(tmp2, ((STYLESTRUCT*)_lParam)->styleNew);
		StyleName = "GWL_EXSTYLE";
	}
	else 
	{
		as_string::style_flags(tmp, ((STYLESTRUCT*)_lParam)->styleOld);
		as_string::style_flags(tmp2, ((STYLESTRUCT*)_lParam)->styleNew);
	}
	snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x %s %s %s -> %s", _hWnd, _MsgName, StyleName, tmp, tmp2);
	return _StrDestination;
}

char* parse_wm_message(char* _StrDestination, size_t _SizeofStrDestination, oWINKEY_CONTROL_STATE* _pState, HWND _hWnd, UINT _uMsg, WPARAM _wParam, LPARAM _lParam)
{
	// http://www.autoitscript.com/autoit3/docs/appendix/WinMsgCodes.htm

	#define KEYSTR ouro::as_string(oWinKeyToKey((DWORD)oWinKeyTranslate(_wParam, _pState)))

	switch (_uMsg)
	{ 
		case WM_ACTIVATE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATE %s, other HWND 0x%x %sactivated", _hWnd, as_string::WA((UINT)_wParam), _lParam, _wParam == WA_INACTIVE ? "" : "de"); break;
		case WM_ACTIVATEAPP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_ACTIVATEAPP %sactivated, other thread %d %sactivated", _hWnd, _wParam ? "" : "de", _lParam, _wParam ? "de" : ""); break;
		case WM_NCACTIVATE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_NCACTIVATE titlebar/icon needs to be changed: %s lParam=%x", _hWnd, _wParam ? "true" : "false", _lParam); break;
		case WM_SETTEXT: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SETTEXT: lParam=%s", _hWnd, _lParam); break;
		case WM_WINDOWPOSCHANGING: { char tmp[1024]; WINDOWPOS& wp = *(WINDOWPOS*)_lParam; snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_WINDOWPOSCHANGING hwndInsertAfter=%x xy=%d,%d wh=%dx%d flags=%s", _hWnd, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, as_string::swp_flags(tmp, wp.flags)); break; }
		case WM_WINDOWPOSCHANGED: { char tmp[1024]; WINDOWPOS& wp = *(WINDOWPOS*)_lParam; snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_WINDOWPOSCHANGED hwndInsertAfter=%x xy=%d,%d wh=%dx%d flags=%s", _hWnd, wp.hwndInsertAfter, wp.x, wp.y, wp.cx, wp.cy, as_string::swp_flags(tmp, wp.flags)); break; }
		case WM_STYLECHANGING: { print_style_change(_StrDestination, _SizeofStrDestination, _hWnd, "WM_STYLECHANGING", _wParam, _lParam); break; }
		case WM_STYLECHANGED: { print_style_change(_StrDestination, _SizeofStrDestination, _hWnd, "WM_STYLECHANGED", _wParam, _lParam); break; }
		case WM_DISPLAYCHANGE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DISPLAYCHANGE %dx%dx%d", _hWnd, static_cast<int>(GET_X_LPARAM(_lParam)), static_cast<int>(GET_Y_LPARAM(_lParam)), _wParam); break;
		case WM_DWMCOLORIZATIONCOLORCHANGED: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMCOLORIZATIONCOLORCHANGED", _hWnd); break;
		case WM_DWMCOMPOSITIONCHANGED: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMCOMPOSITIONCHANGED", _hWnd); break;
		case WM_DWMNCRENDERINGCHANGED: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMNCRENDERINGCHANGED Desktop Window Manager (DWM) %s", _hWnd, (BOOL)_wParam ? "enabled" : "disabled"); break;
		case WM_DWMWINDOWMAXIMIZEDCHANGE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DWMWINDOWMAXIMIZEDCHANGE", _hWnd); break;
		case WM_MOVE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOVE %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SETCURSOR: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SETCURSOR %s hit=%s, id=%s", _hWnd, (HWND)_wParam == _hWnd ? "In Window" : "Out of Window", as_string::HT(GET_X_LPARAM(_lParam)), as_string::WM(GET_Y_LPARAM(_lParam))); break;
		case WM_SHOWWINDOW: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SHOWWINDOW: wParam=%s lParam=%s", _hWnd, _wParam ? "shown" : "hidden", as_string::WMSW(_lParam)); break;
		case WM_SIZE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SIZE %dx%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SYSCOMMAND: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSCOMMAND: wParam=%s screenpos=%d,%d", _hWnd, as_string::SC((UINT)(_wParam&0xfff0)), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_SYSKEYDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYDOWN: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_SYSKEYUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_SYSKEYUP: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_KEYDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYDOWN: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_KEYUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_KEYUP: wParam=%s lParam=%0x", _hWnd, KEYSTR, _lParam); break;
		case WM_MOUSEMOVE: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEMOVE %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_LBUTTONDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_LBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_LBUTTONUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_LBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MBUTTONDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MBUTTONUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_RBUTTONDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_RBUTTONDOWN %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_RBUTTONUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_RBUTTONUP %d,%d", _hWnd, GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_XBUTTONDOWN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_XBUTTONDOWN(%d) %d,%d", _hWnd, GET_XBUTTON_WPARAM(_wParam), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_XBUTTONUP: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_XBUTTONUP(%d) %d,%d", _hWnd, GET_XBUTTON_WPARAM(_wParam), GET_X_LPARAM(_lParam), GET_Y_LPARAM(_lParam)); break;
		case WM_MOUSEWHEEL: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEWHEEL %d", _hWnd, GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_MOUSEHWHEEL: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_MOUSEHWHEEL %d", _hWnd, GET_WHEEL_DELTA_WPARAM(_wParam)); break;
		case WM_INPUT: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_INPUT %s", _hWnd, _wParam == RIM_INPUT ? "RIM_INPUT" : "RIM_INPUTSINK"); break;
		case WM_IME_SETCONTEXT: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_IME_SETCONTEXT window %sactive... display flags 0x%x", _hWnd, _wParam ? "" : "in", _lParam); break;
		case WM_IME_NOTIFY: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_IME_NOTIFY wParam == 0x%x lParam = 0x%x", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORBTN: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORBTN HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORDLG: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORDLG HDC 0x%x Dlghwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLOREDIT: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLOREDIT HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORLISTBOX: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORLISTBOX HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORMSGBOX: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORMSGBOX HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORSCROLLBAR: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORSCROLLBAR HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_CTLCOLORSTATIC: snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_CTLCOLORSTATIC HDC 0x%x DlgItemhwnd = %p", _hWnd, _wParam, _lParam); break;
		case WM_NOTIFY: { const NMHDR& h = *(NMHDR*)_lParam; snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_NOTIFY WPARAM=%u from hwndFrom=0x%x idFrom=%d notification code=%s", _hWnd, _wParam, h.hwndFrom, h.idFrom, as_string::NM(h.code)); break; }
		case WM_DROPFILES:
		{
			path_string p;
			oCHECK_SIZE(UINT, p.capacity());
			UINT nFiles = DragQueryFile((HDROP)_wParam, ~0u, p, static_cast<UINT>(p.capacity()));
			DragQueryFile((HDROP)_wParam, 0, p, static_cast<UINT>(p.capacity()));
			snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DROPFILES hDrop=0x%x %u files starting with \"%s\"", _hWnd, _wParam, nFiles, p.c_str());
			break;
		}
		case WM_INPUT_DEVICE_CHANGE:
		{
			const char* type = _wParam == GIDC_ARRIVAL ? "arrival" : "removal";

			mstring Name;
			oCHECK_SIZE(UINT, Name.capacity() * sizeof(mstring::char_type));
			UINT Size = static_cast<UINT>(Name.capacity() * sizeof(mstring::char_type));
			GetRawInputDeviceInfo((HANDLE)_lParam, RIDI_DEVICENAME, Name.c_str(), &Size);

			RID_DEVICE_INFO RIDDI;
			Size = sizeof(RIDDI);
			GetRawInputDeviceInfo((HANDLE)_lParam, RIDI_DEVICEINFO, &RIDDI, &Size);
			ouro::input::type InpType = ouro::input::unknown;
			switch (RIDDI.dwType)
			{
				case RIM_TYPEKEYBOARD: InpType = ouro::input::keyboard; break;
				case RIM_TYPEMOUSE: InpType = ouro::input::mouse; break;
				default:
				case RIM_TYPEHID: InpType = ouro::input::unknown; break;
			}

			sstring StrType;
			snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_INPUT_DEVICE_CHANGE %s type=%s devname=%s", _hWnd, type, to_string(StrType, InpType), Name.c_str());
			break;
		}

		case WM_DEVICECHANGE: 
		{ 
			DEV_BROADCAST_HDR* pDBHdr = (DEV_BROADCAST_HDR*)_lParam;
			const char* devtype = pDBHdr ? as_string::DBTDT(pDBHdr->dbch_devicetype) : "(null)";

			const guid* pGUID = &null_guid;
			const char* name = "(null)";

			if (pDBHdr)
			{
				switch (pDBHdr->dbch_devicetype)
				{
					case DBT_DEVTYP_DEVICEINTERFACE:
					{
						DEV_BROADCAST_DEVICEINTERFACE_A* pDBDI = (DEV_BROADCAST_DEVICEINTERFACE_A*)_lParam;
						pGUID = (const guid*)&pDBDI->dbcc_classguid;
						name = pDBDI->dbcc_name;
						break;
					}
					case DBT_DEVTYP_HANDLE:
					{
						DEV_BROADCAST_HANDLE* pDBH = (DEV_BROADCAST_HANDLE*)_lParam;
						pGUID = (const guid*)&pDBH->dbch_eventguid;
						break;
					}
					case DBT_DEVTYP_OEM:
					{
						DEV_BROADCAST_OEM* pDBOEM = (DEV_BROADCAST_OEM*)_lParam;
						name = "OEM";
						break;
					}
					case DBT_DEVTYP_PORT:
					{
						DEV_BROADCAST_PORT* pDBPort = (DEV_BROADCAST_PORT*)_lParam;
						name = pDBPort->dbcp_name;
						break;
					}
					case DBT_DEVTYP_VOLUME:
					{
						DEV_BROADCAST_VOLUME* pDBVol = (DEV_BROADCAST_VOLUME*)_lParam;
						switch (pDBVol->dbcv_flags)
						{
							case DBTF_MEDIA: name = "physical media device"; break;
							case DBTF_NET: name = "network device"; break;
							case DBTF_MEDIA|DBTF_NET: name = "network media device"; break;
							default: name = "physical device";
						}
						break;
					}
					default: break;
				}
			}

			sstring StrGUID;
			oCHECK_SIZE(int, _wParam);
			snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x WM_DEVICECHANGE %s devtype=%s GUID=%s name=%s", _hWnd, as_string::DBT(static_cast<int>(_wParam)), devtype, to_string(StrGUID, *pGUID), name);
			break;
		}
		default:
		{
			const char* WMStr = as_string::WM(_uMsg);
			snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x %s", _hWnd, WMStr);
			if (*WMStr == 'u') // unrecognized
				snprintf(_StrDestination, _SizeofStrDestination, "HWND 0x%x Unrecognized uMsg=0x%x (%u)", _hWnd, _uMsg, _uMsg); break;
			break;
		}
	}

	return _StrDestination;
}

	} // namespace windows
} // namespace ouro
