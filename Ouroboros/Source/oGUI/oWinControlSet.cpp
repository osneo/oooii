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
#include <oGUI/Windows/oWinControlSet.h>
#include <oCore/windows/win_error.h>
#include <oBase/algorithm.h>
#include <oGUI/Windows/oGDI.h>
#include <oGUI/Windows/oWinRect.h>
#include <oBasis/oError.h> // @tony fixme

using namespace ouro;

bool oWinControlSet::Initialize(HWND _hParent, const int2& _ParentClientSize, const xml& _XML, const IDFROMSTRING& _IDFromString)
{
	if (!_hParent)
		return oErrorSetLast(std::errc::invalid_argument, "A valid HWND must be specified as the parent");

	xml::node hRoot = _XML.first_child(_XML.root(), "oUI");
	if (!hRoot)
		return oErrorSetLast(std::errc::invalid_argument, "XML root node \"oUI\" not found");

	fonts_t fonts;
	if (!CreateFontsSibling(_XML, &fonts))
		return false; // pass through error

	CONTROL_CONTEXT cc;
	cc.hParent = _hParent;
	cc.ParentSize = _ParentClientSize;
	cc.IDFromString = _IDFromString;

	XML_CONTEXT xc(fonts);
	xc.pXML = &_XML;
	xc.hNode = hRoot;

	if (!CreateControlsSibling(xc, cc, &Controls))
		return false; // pass through error

	return true;
}

void oWinControlSet::Deinitialize()
{
	for (HWND hWnd : Controls)
		if (IsWindow(hWnd))
			DestroyWindow(hWnd);

	Controls.clear();
}

HWND oWinControlSet::GetControl(int _ID) const
{
	if (_ID < 0 || _ID >= as_int(Controls.size()))
		return (HWND)oErrorSetLast(std::errc::invalid_argument, "Invalid ID %d", _ID);
	return Controls[_ID];
}

bool oWinControlSet::CreateFontsSibling(const xml& _XML, fonts_t* _pFonts)
{
	xml::node hRoot = _XML.first_child(_XML.root(), "oUI");
	if (!hRoot)
		return oErrorSetLast(std::errc::invalid_argument, "XML root node \"oUI\" not found");

	xml::node hChild = _XML.first_child(hRoot, "Font");
	while (hChild)
	{
		const char* StrFont = _XML.find_attr_value(hChild, "Name");
		if (!StrFont)
			return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Name attribute in a Font node");

		if (_pFonts->end() != _pFonts->find(StrFont))
			return oErrorSetLast(std::errc::invalid_argument, "Font %s already defined", StrFont);

		ouro::font_info fd;
		if (!ParseFontDesc(_XML, hChild, &fd))
			return false; // pass through error

		HFONT hFont = oGDICreateFont(fd);
		if (!hFont)
			return oErrorSetLast(std::errc::invalid_argument, "Could not create font %s: %s: %s", StrFont, oErrorAsString(oErrorGetLast()), oErrorGetLastString());

		(*_pFonts)[StrFont] = hFont;

		hChild = _XML.next_sibling(hChild, "Font");
	}

	return true;
}

// @tony: Should this be promoted to somewhere more generic?
#define oXML_GETVALUE(_XML, _hNode, _pDestStruct, _FieldName) _XML.find_attr_value(_hNode, #_FieldName, &_pDestStruct->_FieldName);

bool oWinControlSet::ParseFontDesc(const xml& _XML, xml::node _hNode, ouro::font_info* _pDesc)
{
	oXML_GETVALUE(_XML, _hNode, _pDesc, name);
	oXML_GETVALUE(_XML, _hNode, _pDesc, point_size);
	oXML_GETVALUE(_XML, _hNode, _pDesc, bold);
	oXML_GETVALUE(_XML, _hNode, _pDesc, italic);
	oXML_GETVALUE(_XML, _hNode, _pDesc, underline);
	oXML_GETVALUE(_XML, _hNode, _pDesc, strikeout);
	return true;
}

bool oWinControlSet::ParseControlDesc(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, const controls_t& _Controls, ouro::control_info* _pDesc)
{
	_pDesc->parent = (ouro::window_handle)_ControlContext.hParent;
	_pDesc->font = (ouro::font_handle)_ControlContext.hFont;

	const char* StrID = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "ID");
	if (!oSTRVALID(StrID))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing ID attribute for the specified Control.");

	int ID = ouro::invalid;
	if (_ControlContext.IDFromString(&ID, StrID))
	{
		_pDesc->id = as_ushort(ID);
	}
	else
		return oErrorSetLast(std::errc::invalid_argument, "Undeclared ID %s. All IDs must be declared as an enum in code.", StrID);

	if (ID < as_int(_Controls.size()) && _Controls[ID])
		return oErrorSetLast(std::errc::invalid_argument, "ID %s has already been used", StrID);

	if (!_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Type", &_pDesc->type))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Type attribute for Control %s.", StrID);

	_pDesc->text = oSAFESTR(_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Text"));
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "StartsNewGroup", &_pDesc->starts_new_group); // allow this to default to false

	const char* StrFont = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Font"); // allow this to default to the last context's font
	if (StrFont)
	{
		auto it = _XmlContext.Fonts.find(StrFont);
		if (it == _XmlContext.Fonts.end())
			return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Font %s defined in Control %s", StrFont, StrID);
		_pDesc->font = (ouro::font_handle)it->second;
	}

	// Resolve size relative to parent/context
	int2 ControlPos(0, 0);
	if (!_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Pos", &ControlPos))
		return oErrorSetLast(std::errc::invalid_argument, "Invalid or missing Pos attribute for Control %s.", StrID);

	int2 ControlSize(oDEFAULT, oDEFAULT);
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Size", &ControlSize); // allow oDEFAULT to fall through if not specified
	ControlSize = oWinControlGetInitialSize(_pDesc->type, ControlSize);

	ouro::alignment::value alignment = ouro::alignment::top_left;
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Align", &alignment); // allow TopLeft as default value

	RECT rParentClient;
	int2 ParentSize = ouro::resolve_rect_size(_ControlContext.ParentSize, oWinRectSize(rParentClient));
	RECT rParent = oWinRectWH(int2(0,0), ParentSize);

	RECT rControl = oWinRect(ouro::resolve_rect(oRect(rParent), ControlPos, ControlSize, alignment, false));
	_pDesc->position = oWinRectPosition(rControl);
	_pDesc->size = oWinRectSize(rControl);

	return true;
}

void oWinControlSet::AddControl(int _ID, HWND _hHandle)
{
	safe_set(Controls, _ID, _hHandle);
}

HWND oWinControlSet::CreateControl(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls, int2 _ParentOffset)
{
	ouro::control_info d;
	if (!ParseControlDesc(_XmlContext, _ControlContext, *_pControls, &d))
		return nullptr; // pass through error

	bool enabled = _ControlContext.Enabled;
	bool visible = _ControlContext.Visible;
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Enabled", &enabled);
	_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Visible", &visible);

	d.position += _ParentOffset;
	HWND hControl = oWinControlCreate(d);
	oVB(hControl);

	// @tony: there still might be some redraw issues if this is the last control 
	// that may justify moving visible and enabled into control creation to take 
	// advantage of first-draw

	if (!enabled)
		oWinControlEnable(hControl, false);

	if (!visible)
		oWinControlSetVisible(hControl, false);

	safe_set(*_pControls, d.id, hControl);
	return hControl;
}

bool oWinControlSet::CreateControlsSibling(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls)
{
	xml::node hChild = _XmlContext.pXML->first_child(_XmlContext.hNode);
	while (hChild)
	{
		XML_CONTEXT xc = _XmlContext;
		xc.hNode = hChild;
		if (!CreateControlsRecursive(xc, _ControlContext, _pControls))
			return false; // pass through error
		hChild = _XmlContext.pXML->next_sibling(hChild);
	}

	return true;
}

bool oWinControlSet::CreateControlsRecursive(const XML_CONTEXT& _XmlContext, const CONTROL_CONTEXT& _ControlContext, controls_t* _pControls)
{
	CONTROL_CONTEXT Current = _ControlContext;
	const char* NodeName = _XmlContext.pXML->node_name(_XmlContext.hNode);
	if (oSTRVALID(NodeName))
	{
		bool VisitChildren = false;
		int2 RelativePos;
		int2 Size(ouro::invalid, ouro::invalid);
		if (!_stricmp("RefPoint", NodeName))
		{
			//#ifdef _DEBUG
			//	oTRACE("<RefPoint Name=\"%s\" ... >", oSAFESTRN(_XmlContext.pXML->GetValue(_XmlContext.hNode, "Name")));
			//#endif
			  
			if (_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Pos", &RelativePos))
				Current.ParentPosition += RelativePos;

			if (_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Size", &Size))
				Current.ParentSize = Size; 

			const char* StrFont = _XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Font");
			if (StrFont)
				Current.hFont = ((fonts_t&)_XmlContext.Fonts)[StrFont];

			_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Visible", &Current.Visible);
			_XmlContext.pXML->find_attr_value(_XmlContext.hNode, "Enabled", &Current.Enabled);

			VisitChildren = true;
		}

		else if (!_stricmp("Control", NodeName))
		{
			Current.hParent = CreateControl(_XmlContext, Current, _pControls, Current.ParentPosition);
			if (!Current.hParent)
				return false; // pass through error

			RECT rParent = oWinGetRelativeRect(Current.hParent);
			Current.ParentPosition = oWinRectPosition(rParent);
			Current.ParentSize = oWinRectSize(rParent);
			VisitChildren = true;
		}

		if (VisitChildren)
		{
			if (!CreateControlsSibling(_XmlContext, Current, _pControls))
				return false; // pass through error
		}
	}
	return true;
}
