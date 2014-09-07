// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBase/path.h>
#include <oBase/throw.h>

namespace ouro {
	namespace tests {

template<typename charT>
struct CASE
{
	const charT* CtorArgument;
	const charT* Iteration;
	const charT* AsStr;
	const charT* AsGenericStr;
	const charT* RootPath;
	const charT* RootName;
	const charT* RootDir;
	const charT* RelPath;
	const charT* ParentPath;
	const charT* Filename;
};

// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2011/n3239.html#Path-decomposition-table
static const CASE<char> kCases[] = 
{
	{ nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, },
	{ ".", ".", ".", ".", nullptr, nullptr, nullptr, ".", nullptr, ".", },
	{ "..", "..", "..", "..", nullptr, nullptr, nullptr, "..", nullptr, "..", },
	{ "foo", "foo", "foo", "foo", nullptr, nullptr, nullptr, "foo", nullptr, "foo", },
	{ "/", "/", "/", "/", "/", nullptr, "/", nullptr, nullptr, "/", },
	{ "/foo", "/,foo", "/foo", "/foo", "/", nullptr, "/", "foo", "/", "foo" },
	{ "foo/", "foo,.", "foo/", "foo/", nullptr, nullptr, nullptr, "foo/", "foo", "." },
	{ "/foo/", "/,foo,.", "/foo/", "/foo/", "/", nullptr, "/", "foo/", "/foo", "." },
	{ "foo/bar", "foo,bar", "foo/bar", "foo/bar", nullptr, nullptr, nullptr, "foo/bar", "foo", "bar" },
	{ "/foo/bar", "/,foo,bar", "/foo/bar", "/foo/bar", "/", nullptr, "/", "foo/bar", "/foo", "bar" },
	{ "//net", "//net", "//net", "//net", "//net", "//net", nullptr, nullptr, nullptr, "//net" },
	{ "//net/foo", "//net,/,foo", "//net/foo", "//net/foo", "//net/", "//net", "/", "foo", "//net/", "foo" },
	{ "///foo///", "/,foo,.", "///foo///", "///foo///", "/", nullptr, "/", "foo///", "///foo", "." },
	{ "///foo///bar", "/,foo,bar", "///foo///bar", "///foo///bar", "/", nullptr, "/", "foo///bar", "///foo", "bar" },
	{ "/.", "/,.", "/.", "/.", "/", nullptr, "/", ".", "/", "." },
	{ "./", ".,.", "./", "./", nullptr, nullptr, nullptr, "./", ".", "." },
	{ "/..", "/,..", "/..", "/..", "/", nullptr, "/", "..", "/", ".." },
	{ "../", "..,.", "../", "../", nullptr, nullptr, nullptr, "../", "..", "." },
	{ "foo/.", "foo,.", "foo/.", "foo/.", nullptr, nullptr, nullptr, "foo/.", "foo", "." },
	{ "foo/..", "foo,..", "foo/..", "foo/..", nullptr, nullptr, nullptr, "foo/..", "foo", ".." },
	{ "foo/./", "foo,.,.", "foo/./", "foo/./", nullptr, nullptr, nullptr, "foo/./", "foo/.", "." },
	{ "foo/./bar", "foo,.,bar", "foo/./bar", "foo/./bar", nullptr, nullptr, nullptr, "foo/./bar", "foo/.", "bar" },
	{ "foo/..", "foo,..", "foo/..", "foo/..", nullptr, nullptr, nullptr, "foo/..", "foo", ".." },
	{ "foo/../", "foo,..,.", "foo/../", "foo/../", nullptr, nullptr, nullptr, "foo/../", "foo/..", "." },
	{ "foo/../bar", "foo,..,bar", "foo/../bar", "foo/../bar", nullptr, nullptr, nullptr, "foo/../bar", "foo/..", "bar" },
	#if 0//defined(_WIN32) || defined(_WIN64)
		{ "c:", "c:", "c:", "c:", "c:", "c:", nullptr, nullptr, nullptr, "c:" },
		{ "c:/", "c:,/", "c:/", "c:/", "c:/", "c:", "/", nullptr, "c:", "/" },
		{ "c:foo", "c:,foo", "c:foo", "c:foo", "c:", "c:", nullptr, "foo", "c:", "foo" },
		{ "c:/foo", "c:,/,foo", "c:/foo", "c:/foo", "c:/", "c:", "/", "foo", "c:/", "foo" },
		{ "c:foo/", "c:,foo,.", "c:foo/", "c:foo/", "c:", "c:", nullptr, "foo/", "c:foo", "." },
		{ "c:/foo/", "c:,/,foo,.", "c:/foo/", "c:/foo/", "c:/", "c:", "/", "foo/", "c:/foo", "." },
		{ "c:/foo/bar", "c:,/,foo,bar", "c:/foo/bar", "c:/foo/bar", "c:/", "c:", "/", "foo/bar", "c:/foo", "bar" },
		{ "prn:", "prn:", "prn:", "prn:", "prn:", "prn:", nullptr, nullptr, nullptr, "prn:" },
		{ "c:\\", "c:,/", "c:\\", "c:/", "c:\\", "c:", "\\", nullptr, "c:", "\\" },
		{ "c:\\foo", "c:,/,foo", "c:\\foo", "c:/foo", "c:\\", "c:", "\\", "foo", "c:\\", "foo" },
		{ "c:foo\\", "c:,foo,.", "c:foo\\", "c:foo/", "c:", "c:", nullptr, "foo\\", "c:foo", "." },
		{ "c:\\foo\\", "c:,/,foo,.", "c:\\foo\\", "c:/foo/", "c:\\", "c:", "\\", "foo\\", "c:\\foo", "." },
		{ "c:\\foo/", "c:,/,foo,.", "c:\\foo/", "c:/foo/", "c:\\", "c:", "\\", "foo/", "c:\\foo", "." },
		{ "c:/foo\\bar", "c:,/,foo,bar", "c:/foo\\bar", "c:/foo/bar", "c:/", "c:", "/", "foo\\bar", "c:/foo", "bar" },
	#else
		{ "c:", "c:", "c:", "c:", nullptr, nullptr, nullptr, "c:", nullptr, "c:" },
		{ "c:/", "c:,.", "c:/", "c:/", nullptr, nullptr, nullptr, "c:/", "c:", "." },
		{ "c:foo", "c:foo", "c:foo", "c:foo", nullptr, nullptr, nullptr, "c:foo", nullptr, "c:foo" },
		{ "c:/foo", "c:,foo", "c:/foo", "c:/foo", nullptr, nullptr, nullptr, "c:/foo", "c:", "foo" },
		{ "c:foo/", "c:foo,.", "c:foo/", "c:foo/", nullptr, nullptr, nullptr, "c:foo/", "c:foo", "." },
		{ "c:/foo/", "c:,foo,.", "c:/foo/", "c:/foo/", nullptr, nullptr, nullptr, "c:/foo/", "c:/foo", "." },
		{ "c:/foo/bar", "c:,foo,bar", "c:/foo/bar", "c:/foo/bar", nullptr, nullptr, nullptr, "c:/foo/bar", "c:/foo", "bar" },
		{ "prn:", "prn:", "prn:", "prn:", nullptr, nullptr, nullptr, "prn:", nullptr, "prn:" },
		{ "c:\\", "c:\\", "c:\\", "c:\\", nullptr, nullptr, nullptr, "c:\\", nullptr, "c:\\" },
		{ "c:\\foo", "c:\\foo", "c:\\foo", "c:\\foo", nullptr, nullptr, nullptr, "c:\\foo", nullptr, "c:\\foo" },
		{ "c:foo\\", "c:foo\\", "c:foo\\", "c:foo\\", nullptr, nullptr, nullptr, "c:foo\\", nullptr, "c:foo\\" },
		{ "c:\\foo\\", "c:\\foo\\", "c:\\foo\\", "c:\\foo\\", nullptr, nullptr, nullptr, "c:\\foo\\", nullptr, "c:\\foo\\" },
		{ "c:\\foo/", "c:\\foo,.", "c:\\foo/", "c:\\foo/", nullptr, nullptr, nullptr, "c:\\foo/", "c:\\foo", "." },
		{ "c:/foo\\bar", "c:,foo\bar", "c:/foo\\bar", "c:/foo\\bar", nullptr, nullptr, nullptr, "c:/foo\\bar", "c:", "foo\\bar" },
	#endif
};

template<typename charT>
struct boost_and_std_proposal_compliant : path_traits<charT, true, false> {};

typedef basic_path<char, boost_and_std_proposal_compliant<char>> test_path;

static void TESTpath_standard_cases()
{
	int i = 0;
	for (const auto& c : kCases)
	{
		try
		{
			test_path P(c.CtorArgument);

			#define TEST_CASE(_API, _Field) do \
			{	auto test = P._API(); \
				if (c._Field == nullptr && P.has_##_API()) \
					oTHROW(protocol_error, "\"%s\"." "has_" #_API "() returned true when it should be false (thinks its \"%s\")", c.CtorArgument ? c.CtorArgument : "(empty)", test.c_str()); \
				if (c._Field != nullptr && strcmp(test, c._Field)) \
					oTHROW(protocol_error, "\"%s\"." #_API "(): expected \"%s\", got \"%s\"" \
						, c.CtorArgument ? c.CtorArgument : "(empty)" \
						, c._Field ? c._Field : "empty" \
						, test.empty() ? "(empty)" : test.c_str()); \
			} while (false)

			//TEST_ITER();

			path::string_type test = P.string();
			if (!c.AsStr && !test.empty())
				oTHROW(protocol_error, "string() should have an empty value instead of \"%s\"", test.c_str());

			if (c.AsStr && strcmp(test, c.AsStr))
				oTHROW(protocol_error, "\"%s\".string(): expected \"%s\", got \"%s\""
					, test.c_str(), c.AsStr ? c.AsStr : "empty"
					, test.empty() ? "(empty)" : test.c_str());

			//TEST_CASE(generic_string, AsGenericStr);
			TEST_CASE(root_path, RootPath);
			TEST_CASE(root_name, RootName);
			TEST_CASE(root_directory, RootDir);
			TEST_CASE(relative_path, RelPath);
			TEST_CASE(parent_path, ParentPath);
			TEST_CASE(filename, Filename);
		}

		catch (std::exception& e)
		{
			oTHROW(protocol_error, "path test %d failed: %s", i, e.what());
		}

		i++;
	}
}

template<typename charT>
struct CASE2
{
	const charT* CtorArgument;
	const charT* Base;
	const charT* Ext;
	const charT* Stem;
};

static const CASE2<char> kMoreCases[] = 
{
	{ nullptr, nullptr, nullptr, nullptr },
	{ "", nullptr, nullptr, nullptr },
	{ "c:/foo/bar/base.ext", "base", ".ext", "c:/foo/bar/base" },
	{ "c:/foo/bar/.ext", nullptr, ".ext", "c:/foo/bar/" },
	{ "c:/foo/bar/base", "base", nullptr, "c:/foo/bar/base" },
	{ "c:/foo/bar/base.not-ext.not-ext.ext", "base.not-ext.not-ext", ".ext", "c:/foo/bar/base.not-ext.not-ext" },
};

static void TESTpath_more_cases()
{
	for (auto i = 0; i < oCOUNTOF(kMoreCases); i++)
	{
		try
		{
			const CASE2<char>& c = kMoreCases[i];
 			test_path P(c.CtorArgument);

			#define TEST_CASE2(_API, _Field) do \
			{ auto test = P._API(); \
				if (!c._Field && !test.empty()) \
					oTHROW(protocol_error, #_API "() should have an empty value instead of \"%s\"", test.c_str()); \
				if (c._Field && strcmp(test, c._Field)) \
					oTHROW(protocol_error, "\"%s\"." #_API "(): expected \"%s\", got \"%s\"" \
						, test.c_str(), c._Field ? c._Field : "empty" \
						, test.empty() ? "(empty)" : test.c_str()); \
			} while (false)

			TEST_CASE2(basename, Base);
			TEST_CASE2(extension, Ext);
			TEST_CASE2(stem, Stem);
		}

		catch (std::exception& e)
		{
			oTHROW(protocol_error, "path test2 %d failed: %s", i, e.what());
		}
	}
}

template<typename charT>
struct CLEAN_CASE
{
	const charT* CtorArgument;
	const charT* Clean;
};

static const CLEAN_CASE<char> kCleanCases[] = 
{
	{ "c:/my//path", "c:/my/path" },
	{ "//c/my/path", "//c/my/path" },
	{ "C:/AutoBuild/1.0.002.6398//../WebRoot/26417/oUnitTests.txt.stderr", "C:/AutoBuild/WebRoot/26417/oUnitTests.txt.stderr" },
};

static void TESTpath_clean()
{
	for (auto i = 0; i < oCOUNTOF(kCleanCases); i++)
	{
		try
		{
			const CLEAN_CASE<char>& c = kCleanCases[i];
			path P(c.CtorArgument);
			if (strcmp(P, c.Clean))
				oTHROW(protocol_error, "\"%s\".clean() expected \"%s\", got \"%s\"", c.CtorArgument, c.Clean, P.c_str());
		}

		catch (std::exception& e)
		{
			oTHROW(protocol_error, "path test2 %d failed: %s", i, e.what());
		}
	}
}

void TESTpath()
{
	TESTpath_standard_cases();
	TESTpath_more_cases();
	TESTpath_clean();

	test_path P("c:/foo/bar/img.png");

	P.replace_extension(".jpg");
	if (strcmp(P, "c:/foo/bar/img.jpg"))
		oTHROW(protocol_error, "replace_extension failed");

	P.replace_extension("bmp");
	if (strcmp(P, "c:/foo/bar/img.bmp"))
		oTHROW(protocol_error, "replace_extension failed");

	P.replace_filename("file.txt");
	if (strcmp(P, "c:/foo/bar/file.txt"))
		oTHROW(protocol_error, "replace_filename failed");

	P.remove_filename();
	if (strcmp(P, "c:/foo/bar/"))
		oTHROW(protocol_error, "remove_leaf failed");

	P.remove_filename();
	if (strcmp(P, "c:/foo/bar"))
		oTHROW(protocol_error, "remove_leaf failed");

	P = "/";
	P.remove_filename();
	if (!P.empty())
		oTHROW(protocol_error, "remove_leaf failed");
}

	} // namespace tests
} // namespace ouro
