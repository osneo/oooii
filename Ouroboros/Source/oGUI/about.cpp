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
#include <oGUI/about.h>
#include <oGUI/window.h>
#include <oGUI/Windows/oWinRect.h>
#include <oGUI/Windows/oWinWindowing.h>
#include <oCore/module.h>

static void show_modal(const std::shared_ptr<ouro::window>& _Parent, const std::shared_ptr<ouro::basic_window>& _Child, bool _Show)
{
	_Parent->enabled(!_Show);
	_Child->owner(_Show ? _Parent : nullptr);
	_Child->show(_Show ? ouro::window_state::restored : ouro::window_state::hidden);
}

namespace ouro {

class about_impl : public about
{
public:
	about_impl(const info& _Info);

	// basic_window API
	ouro::window_handle native_handle() const override { return Window->native_handle(); }
	display::id display_id() const override { return Window->display_id(); }
	bool is_window_thread() const override { return Window->is_window_thread(); }
	void flush_messages(bool _WaitForNext = false) override { Window->flush_messages(_WaitForNext); }
	void quit() override { Window->quit(); }
	void debug(bool _Debug) override { Window->debug(_Debug); }
	bool debug() const override { return Window->debug(); }
	void state(window_state::value _State) override { Window->state(_State); }
	window_state::value state() const override { return Window->state(); }
	void client_position(const int2& _ClientPosition) override { Window->client_position(_ClientPosition); }
	int2 client_position() const override { return Window->client_position(); }
	int2 client_size() const override { return Window->client_size(); }
	void icon(ouro::icon_handle _hIcon) override { Window->icon(_hIcon); }
	ouro::icon_handle icon() const override { return Window->icon(); }
	void user_cursor(ouro::cursor_handle _hCursor) override { Window->user_cursor(_hCursor); }
	ouro::cursor_handle user_cursor() const override { return Window->user_cursor(); }
	void client_cursor_state(cursor_state::value _State) override { Window->client_cursor_state(_State); }
	cursor_state::value client_cursor_state() const override { return Window->client_cursor_state(); }
	void set_titlev(const char* _Format, va_list _Args) override { Window->set_titlev(_Format, _Args); }
	char* get_title(char* _StrDestination, size_t _SizeofStrDestination) const override { return Window->get_title(_StrDestination, _SizeofStrDestination); }
	void parent(const std::shared_ptr<basic_window>& _Parent) override { Window->parent(_Parent); }
	std::shared_ptr<basic_window> parent() const override { return Window->parent(); }
	void owner(const std::shared_ptr<basic_window>& _Owner) override { Window->owner(_Owner); }
	std::shared_ptr<basic_window> owner() const override { return Window->owner(); }
	void sort_order(window_sort_order::value _SortOrder) override { Window->sort_order(_SortOrder); }
	window_sort_order::value sort_order() const override { return Window->sort_order(); }
	void focus(bool _Focus) override { Window->focus(_Focus); }
	bool has_focus() const override { return Window->has_focus(); }

	// about API
	void show_modal(const std::shared_ptr<window>& _Parent) override;
	
private:
	std::shared_ptr<window> Window;
	std::shared_ptr<window> Parent;
	std::vector<std::string> ComponentComments;

	enum AB_CONTROL
	{
		AB_ICON,
		AB_INSTALL_PATH,
		AB_OK,
		AB_MODULE_INFO,
		AB_WEBSITE,
		AB_ISSUE_SITE,
		AB_COMPONENT_GROUP,
		AB_COMPONENT_LIST,
		AB_COMPONENT_COMMENT,
		AB_CONTROL_COUNT,
	};

	std::array<ouro::window_handle, AB_CONTROL_COUNT> Controls;

private:
	
	struct init
	{
		init(const info& _Info, const module::info& _ModuleInfo)
			: info(_Info)
			, module_info(_ModuleInfo)
		{}

		const struct info& info;
		const module::info& module_info;
	};

	void on_event(const window::basic_event& _Event);
	void on_action(const ouro::input::action& _Action);
	void make_controls(const window::create_event& _CreateEvent);
};

static const int kComponentHeight = 100;

about_impl::about_impl(const info& _Info)
{
	module::info mi = this_module::get_info();

	init Init(_Info, mi);

	sstring Title;
	snprintf(Title, "About %s", mi.product_name.c_str());

	Controls.fill(nullptr);
	
	window::init i;
	i.title = Title;
	i.icon = nullptr;
	i.create_user_data = &Init;
	i.on_action = std::bind(&about_impl::on_action, this, std::placeholders::_1);
	i.on_event = std::bind(&about_impl::on_event, this, std::placeholders::_1);
	i.shape.state = window_state::hidden;
	i.shape.style = window_style::dialog;
	i.shape.client_size = int2(350, 150);

	if (oSTRVALID(_Info.components))
		i.shape.client_size.y += kComponentHeight;
	
	Window = window::make(i);
}

std::shared_ptr<about> about::make(const info& _Info)
{
	return std::make_shared<about_impl>(_Info);
}

void about_impl::make_controls(const window::create_event& _CreateEvent)
{
	const init& Init = *(const init*)_CreateEvent.user;
	const info& Info = Init.info;
	const module::info& mi = Init.module_info;

	const int2 kInset(10, 10);
	
	const int2 kIconSize(75, 75);
	const int2 kIconPos(0, 0);

	const int2 kModulePathSize(_CreateEvent.shape.client_size.x - kInset.x * 2, 30);
	const int2 kModulePathPos(kInset.x, kIconPos.y + kIconSize.y);
	
	const int2 kModuleInfoSize(220, 40);
	const int2 kModuleInfoPos(kIconPos.x + kIconSize.x, kInset.y);
	
	const int2 kWebsiteSize(75, 23);
	const int2 kWebsitePos(kModuleInfoPos.x, kModuleInfoPos.y + kModuleInfoSize.y);

	const int2 kIssueSiteSize(75, 23);
	const int2 kIssueSitePos(_CreateEvent.shape.client_size.x - kIssueSiteSize.x - kInset.x, kWebsitePos.y);

	const int2 kComponentGroupSize(_CreateEvent.shape.client_size.x - kInset.x * 2, kComponentHeight);
	//const int2 kComponentGroupPos(kInset.x, kIconPos.y + kIconSize.y + 20);
	const int2 kComponentGroupPos(kInset.x, kModulePathPos.y + kModulePathSize.y);

	const int2 kComponentListSize(90, kComponentGroupSize.y - 21);
	const int2 kComponentListPos(kComponentGroupPos.x + 5, kComponentGroupPos.y + 15);

	const int2 kComponentCommentSize(kComponentGroupSize.x - kComponentListSize.x - 15, kComponentListSize.y);
	const int2 kComponentCommentPos(kComponentListPos.x + kComponentListSize.x + 5, kComponentListPos.y);

	const int2 kButtonSize(75, 23);

	const ouro::rect rParent = oRect(oWinRectWH(int2(0,0), _CreateEvent.shape.client_size));
	
	ouro::control_info Descs[AB_CONTROL_COUNT];

	// Icon
	{
		ouro::rect rChild = oRect(oWinRectWH(int2(0,0), kIconSize));
		ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

		Descs[AB_ICON].type = control_type::icon;
		Descs[AB_ICON].icon = Info.icon;
		Descs[AB_ICON].position = oWinRectPosition(oWinRect(r));
		Descs[AB_ICON].size = oWinRectSize(oWinRect(r));
	}

	// Install Path
	lstring StrModulePath;
	{
		path ModulePath = this_module::get_path();

		snprintf(StrModulePath, "Install Path: <a href=\"file://%s\">%s</a>", ModulePath.parent_path().c_str(), ModulePath.c_str());

		ouro::rect rChild = oRect(oWinRectWH(kModulePathPos, kModulePathSize));
		ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

		Descs[AB_INSTALL_PATH].type = control_type::hyperlabel;
		Descs[AB_INSTALL_PATH].text = StrModulePath;
		Descs[AB_INSTALL_PATH].position = oWinRectPosition(oWinRect(r));
		Descs[AB_INSTALL_PATH].size = oWinRectSize(oWinRect(r));
	}

	// Module Info
	xlstring StrModuleInfo;
	{
		sstring StrVersion;
		snprintf(StrModuleInfo, "%s%s%s\n%s%s%s%s%s\n%s"
			, mi.product_name.c_str()
			, mi.is_64bit_binary ? "" : " 32-bit"
			, mi.is_debug ? " debug" : ""
			, to_string(StrVersion, mi.version)
			, mi.is_prerelease ? " pre-release" : ""
			, mi.is_patched ? " patched" : ""
			, mi.is_private ? " private" : ""
			, mi.is_special ? " special" : ""
			, mi.copyright.c_str());

		ouro::rect rChild = oRect(oWinRectWH(kModuleInfoPos, kModuleInfoSize));
		ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

		Descs[AB_MODULE_INFO].type = control_type::label;
		Descs[AB_MODULE_INFO].text = StrModuleInfo;
		Descs[AB_MODULE_INFO].position = oWinRectPosition(oWinRect(r));
		Descs[AB_MODULE_INFO].size = oWinRectSize(oWinRect(r));

		//struct version version;
		//mstring company;
		//mstring description;
		//mstring product_name;
		//mstring copyright;
		//mstring original_filename;
		//mstring comments;
		//mstring private_message;
		//mstring special_message;
	}

	// Website
	lstring StrWebsite;
	{
		if (oSTRVALID(Info.website))
		{
			snprintf(StrWebsite, "<a href=\"%s\">visit our website</a>", Info.website);

			ouro::rect rChild = oRect(oWinRectWH(kWebsitePos, kWebsiteSize));
			ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

			Descs[AB_WEBSITE].type = control_type::hyperlabel;
			Descs[AB_WEBSITE].text = StrWebsite;
			Descs[AB_WEBSITE].position = oWinRectPosition(oWinRect(r));
			Descs[AB_WEBSITE].size = oWinRectSize(oWinRect(r));
		}
	}

	// Issue Site
	lstring StrIssueSite;
	{
		if (oSTRVALID(Info.issue_site))
		{
			snprintf(StrIssueSite, "<a href=\"%s\">report an issue</a>", Info.issue_site);

			ouro::rect rChild = oRect(oWinRectWH(kIssueSitePos, kIssueSiteSize));
			ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

			Descs[AB_ISSUE_SITE].type = control_type::hyperlabel;
			Descs[AB_ISSUE_SITE].text = StrIssueSite;
			Descs[AB_ISSUE_SITE].position = oWinRectPosition(oWinRect(r));
			Descs[AB_ISSUE_SITE].size = oWinRectSize(oWinRect(r));
		}
	}

	// OK Button
	{
		ouro::rect rChild = oRect(oWinRectWH(-kInset, kButtonSize));
		ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::bottom_right, true);

		Descs[AB_OK].type = control_type::button;
		Descs[AB_OK].text = "&OK";
		Descs[AB_OK].position = oWinRectPosition(oWinRect(r));
		Descs[AB_OK].size = oWinRectSize(oWinRect(r));
	}

	// Component Group
	{
		if (oSTRVALID(Info.components))
		{
			ouro::rect rChild = oRect(oWinRectWH(kComponentGroupPos, kComponentGroupSize));
			ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

			Descs[AB_COMPONENT_GROUP].type = control_type::group;
			Descs[AB_COMPONENT_GROUP].text = "3rd-Party Components";
			Descs[AB_COMPONENT_GROUP].position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_GROUP].size = oWinRectSize(oWinRect(r));
		}
	}
	
	// Component List
	{
		if (oSTRVALID(Info.components))
		{
			ouro::rect rChild = oRect(oWinRectWH(kComponentListPos, kComponentListSize));
			ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

			Descs[AB_COMPONENT_LIST].type = control_type::listbox;
			Descs[AB_COMPONENT_LIST].text = Info.components;
			Descs[AB_COMPONENT_LIST].position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_LIST].size = oWinRectSize(oWinRect(r));

			tokenize(ComponentComments, Info.component_comments, "|");
		}
	}

	// Component Comment
	{
		if (oSTRVALID(Info.components))
		{
			ouro::rect rChild = oRect(oWinRectWH(kComponentCommentPos, kComponentCommentSize));
			ouro::rect r = ouro::resolve_rect(rParent, rChild, alignment::top_left, true);

			Descs[AB_COMPONENT_COMMENT].type = control_type::hyperlabel;
			Descs[AB_COMPONENT_COMMENT].text = ComponentComments[0].c_str();
			Descs[AB_COMPONENT_COMMENT].position = oWinRectPosition(oWinRect(r));
			Descs[AB_COMPONENT_COMMENT].size = oWinRectSize(oWinRect(r));
		}
	}

	for (short i = 0; i < AB_CONTROL_COUNT; i++)
	{
		if (Descs[i].type != control_type::unknown)
		{
			Descs[i].parent = _CreateEvent.window;
			Descs[i].id = i;
			Controls[i] = (ouro::window_handle)oWinControlCreate(Descs[i]);
		}
		else
			Controls[i] = nullptr;
	}
}

void about_impl::on_event(const window::basic_event& _Event)
{
	switch (_Event.type)
	{
		case event_type::creating:
		{
			make_controls(_Event.as_create());
			break;
		}

		default:
			break;
	}
}

void about_impl::on_action(const ouro::input::action& _Action)
{
	switch (_Action.action_type)
	{
		case input::action_type::control_activated:
			switch (_Action.device_id)
			{
				case AB_OK:
				{
					::show_modal(Parent, Window, false);
					Parent = nullptr;
					quit();
					break;
				}

				case AB_COMPONENT_LIST:
				{
					int index = oWinControlGetSelectedSubItem((HWND)_Action.window);
					if (index >= 0)
					{
						const char* comment = "";
						if (index < static_cast<int>(ComponentComments.size()))
							comment = ComponentComments[index].c_str();
						oWinControlSetText((HWND)Controls[AB_COMPONENT_COMMENT], comment);
					}
					break;
				}

				default:
					break;
			}
			break;

		default:
			break;
	}
}

void about_impl::show_modal(const std::shared_ptr<window>& _Parent)
{
	Parent = _Parent;
	Window->icon(Parent->icon());
	::show_modal(Parent, Window, true);
	Window->flush_messages(true);
	Window->icon(nullptr);
}

} // namespace ouro
