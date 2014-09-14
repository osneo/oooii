// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oTest.h>
#include <oCore/windows/win_registry.h>

using namespace ouro::windows;

namespace ouro {
	namespace tests {

void TESTwin_registry()
{
	static const registry::hkey hKey = registry::current_user;
	static const char* KeyPath = "Software/Ouroboros/oUnitTests/TESTwin_registry";
	static const char* ValueName = "TestValue";
	static const char* TestValue = "Value";
	registry::set(hKey, KeyPath, ValueName, TestValue);
	lstring data;
	bool failed = false;
	try { registry::get(data, hKey, KeyPath, "Non-existant-ValueName"); }
	catch (std::exception&) { failed = true; }
	oCHECK(failed, "succeeded reading a non-existant key %s/%s", KeyPath, ValueName);
	oCHECK0(registry::get(data, hKey, KeyPath, ValueName));
	oCHECK(strcmp(data, TestValue) == 0, "Set/Read values do not match");
	registry::delete_value(hKey, KeyPath, ValueName);
	registry::delete_key(hKey, "Software/Ouroboros/oUnitTests");
}

	}
}
