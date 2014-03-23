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
#include <oBase/scc.h>
#include <oBase/throw.h>
//#include "scc_git.h"
//#include "scc_p4.h"
#include "scc_svn.h"

namespace ouro {

const char* as_string(const scc_protocol::value& _Protocol)
{
	switch (_Protocol)
	{
		case scc_protocol::p4: return "perforce";
		case scc_protocol::svn: return "svn";
		case scc_protocol::git: return "git";
		default: break;
	}
	return "unrecognized scc_protocol";
}

const char* as_string(const scc_status::value& _Status)
{
	switch (_Status)
	{
		case scc_status::unknown: return "unknown";
		case scc_status::unchanged: return "unchanged";
		case scc_status::unversioned: return "unversioned";
		case scc_status::ignored: return "ignored";
		case scc_status::modified: return "modified";
		case scc_status::missing: return "missing";
		case scc_status::added: return "added";
		case scc_status::removed: return "removed";
		case scc_status::replaced: return "replaced";
		case scc_status::copied: return "copied";
		case scc_status::conflict: return "conflict";
		case scc_status::merged: return "merged";
		case scc_status::obstructed: return "obstructed";
		case scc_status::out_of_date: return "out_of_date";
		default: break;
	}
	return "unrecognized scc_status";
}

namespace detail {

class scc_category_impl : public std::error_category
{
public:
	const char* name() const override { return "future"; }
	std::string message(int _ErrCode) const override
	{
		switch (_ErrCode)
		{
			case scc_error::none: return "no error";
			case scc_error::command_string_too_long: return "the command string is too long for internal buffers";
			case scc_error::scc_exe_not_available: return "scc executable not available";
			case scc_error::scc_exe_error: return "scc exe reported an error";
			case scc_error::scc_server_not_available: return "scc server not available";
			case scc_error::file_not_found: return "file not found";
			case scc_error::entry_not_found: return "entry not found";
			default: break;
		}
		return "unrecognized scc error code";
	}
};

} // namespace detail

const std::error_category& scc_category()
{
	static detail::scc_category_impl sSingleton;
	return sSingleton;
}

std::shared_ptr<scc> make_scc(scc_protocol::value _Protocol, const scc_spawn& _Spawn, unsigned int _TimeoutMS)
{
	switch (_Protocol)
	{
		case scc_protocol::svn: return detail::make_scc_svn(_Spawn, _TimeoutMS);
		//case scc_protocol::perforce: return detail::make_scc_p4(_Spawn, _TimeoutMS);
		//case scc_protocol::git: return detail::make_scc_git(_Spawn, _TimeoutMS);
		default: break;
	}
	oTHROW0(protocol_not_supported);
}

} // namespace ouro