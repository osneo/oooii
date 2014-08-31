// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oCore/filesystem.h>
#include <oCore/windows/win_iocp.h>
#include <oBase/assert.h>
#include <oMemory/byte.h>
#include <oBase/event.h>
#include <oBase/finally.h>
#include <oBase/guid.h>
#include <oBase/murmur3.h>
#include <oBase/timer.h>

#include "../../test_services.h"

using namespace ouro::filesystem;

namespace ouro {
	namespace tests {

static void TESTfilesystem_paths()
{
	path path = current_path();
	oTRACE("CWD: %s", path.c_str());
	path = app_path();
	oTRACE("APP: %s", path.c_str());
	path = temp_path();
	oTRACE("SYSTMP: %s", path.c_str());
	path = system_path();
	oTRACE("SYS: %s", path.c_str());
	path = os_path();
	oTRACE("OS: %s", path.c_str());
	path = dev_path();
	oTRACE("DEV: %s", path.c_str());
	path = desktop_path();
	oTRACE("DESKTOP: %s", path.c_str());
	path = data_path();
	oTRACE("DATA: %s", path.c_str());
}

static void TESTfilesystem_read(test_services& _Services)
{
	static const unsigned int kNumReads = 5;

	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "oooii.ico";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	scoped_file ReadFile(TestPath, open_option::binary_read);
	const auto FileSize = file_size(ReadFile);

	size_t BytesPerRead = static_cast<size_t>(FileSize) / kNumReads;
	int ActualReadCount = kNumReads + (((FileSize % kNumReads) > 0) ? 1 : 0);

	char TempFileBlob[1024 * 32];
	void* pHead = TempFileBlob;

	size_t r = 0;
	long long Offset = 0;
	for (int i = 0; i < kNumReads; ++i, r += BytesPerRead)
	{
		seek(ReadFile, Offset, seek_origin::set);
		auto BytesRead = read(ReadFile, pHead, BytesPerRead, BytesPerRead);
		oCHECK(BytesRead == BytesPerRead, "failed to read correctly");
		pHead = ouro::byte_add(pHead, BytesPerRead);
		Offset += BytesPerRead;
	}

	auto RemainingBytes = FileSize - r;
	if (RemainingBytes > 0)
		read(ReadFile, pHead, byte_diff(pHead, TempFileBlob), RemainingBytes);

	static const uint128 ExpectedFileHash(13254728276562583748ull, 8059648572410507760ull);
	oCHECK(murmur3(TempFileBlob, static_cast<unsigned int>(FileSize)) == ExpectedFileHash, "Test failed to compute correct hash");
}

static void TESTfilesystem_write()
{
	path TempFilePath = temp_path() / "TESTfilesystem_write.bin";
	remove(TempFilePath);
	oCHECK(!exists(TempFilePath), "remove failed: %s", TempFilePath.c_str());

	static const guid TestGUID = { 0x9aab7fc7, 0x6ad8, 0x4260, { 0x98, 0xef, 0xfd, 0x93, 0xda, 0x8e, 0xdc, 0x3c } };

	{
		scoped_file WriteFile(TempFilePath, open_option::binary_write);

		auto BytesWritten = write(WriteFile, &TestGUID, sizeof(TestGUID));
		oCHECK(BytesWritten == sizeof(TestGUID), "write failed");
	}

	scoped_allocation LoadedGUID = load(TempFilePath);
	oCHECK(*(guid*)LoadedGUID == TestGUID, "Write failed to write correct guid");
}

static void TESTfilesystem_map(test_services& _Services)
{
	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "Test/Textures/lena_1.png";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	unsigned long long size = file_size(TestPath);
	
	void* mapped = map(TestPath, map_option::binary_read, 0, size);
	if (!mapped)
		oTHROW(protocol_error, "map failed");
	finally Unmap([&] { if (mapped) unmap(mapped); }); // safety unmap if we fail for some non-mapping reason

	scoped_allocation loaded = load(TestPath);

	if (loaded.size() != size)
		oTHROW(io_error, "mismatch: mapped and loaded file sizes");

	if (memcmp(loaded, mapped, loaded.size()))
		oTHROW(protocol_error, "memcmp failed between mapped and loaded files");
		
	unmap(mapped);
	mapped = nullptr; // signal Unmap to noop
}

void TESTfilesystem_async1(test_services& _Services)
{
	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "Test/Geometry/buddha.obj";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());

	event e;
	timer t;
	for (int i = 0; i < 32; i++)
	{
		load_async(TestPath, [&,i](const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)
		{
			if (_pError)
				return;
			e.set(1<<i);
		});
	}

	double after = t.milliseconds();
	oCHECK(e.wait_for (std::chrono::seconds(20), ~0u), "timed out");
	double done = t.milliseconds();

	_Services.report("all dispatches: %.02fms | all completed: %.02fms", after, done);
}

void TESTfilesystem_async2(test_services& _Services)
{
	path_string StrTestPath;
	path TestPath = _Services.test_root_path(StrTestPath, StrTestPath.capacity());
	TestPath /= "oooii.ico";
	if (!exists(TestPath))
		oTHROW(no_such_file_or_directory, "not found: %s", TestPath.c_str());
	event e;
	scoped_allocation p;
	load_async(TestPath, [&](const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)
	{
		if (_pError)
			throw *_pError;
		p = std::move(_Buffer);
		e.set();
	});

	e.wait_for(std::chrono::seconds(10));

	static const uint128 kExpectedFileHash(13254728276562583748ull, 8059648572410507760ull);
	oCHECK(murmur3(p, static_cast<unsigned int>(p.size())) == kExpectedFileHash, "Test failed to compute correct hash" );
}

void TESTfilesystem_async_save(test_services& _Services)
{
	const unsigned int TESTAsyncWrite[] = 
	{ 
		0x474e5089,0x0a1a0a0d,0x0d000000,0x52444849,0x80000000,0x40000000,0x00000608,0x7fd6d200,0x0000007f,0x47527301,
		0xceae0042,0x0000e91c,0x4b620600,0xff004447,0xff00ff00,0x93a7bda0,0x09000000,0x73594870,0x130b0000,0x130b0000,
		0x9c9a0001,0x00000018,0x4d497407,0x05db0745,0x011b0215,0xf341bfc6,0x1d000000,0x74585469,0x6d6d6f43,0x00746e65,
		0x00000000,0x61657243,0x20646574,0x68746977,0x4d494720,0x652e6450,0x03000007,0x4144496c,0xedda7854,0xc3b6d95d,
		0x398c0820,0xe65ffff9,0xf1ad34be,0x6d02e2ca,0xdb5bcccc,0x80e29054,0x1b44d026,0x163c06d1,0x07004c3b,0xe38c1e00,
		0x182a2af2,0xc87de624,0x060f970f,0x10000840,0x90ec2e02,0xff446c5e,0x470aa7de,0xb3f7d771,0x58f1dcd3,0xd7cc9439,
		0x60cf46d2,0xa704c480,0x25b4fdf3,0x6727192c,0x5748716c,0xecc56806,0xdab511d5,0x11955128,0xc8ed5e56,0x067e8001,
		0x21aa8da0,0xf9d98a40,0x33cfd5f5,0x64573bda,0x5afe5e39,0xcfd0af1d,0xecd7356b,0xb357d350,0xd2fb4955,0x48d72ddc,
		0x4ce8b9f6,0x1cef9b1f,0x80f96a05,0xbae92066,0x8e57989e,0xcaf6bda7,0xb176ac59,0x97a117e8,0x3d2533a8,0xde2ab404,
		0x38cb4ab2,0x6d21616c,0x99ae5b55,0x64d4ab95,0x672b56d5,0xb1744a13,0x56eea8c1,0x0f32e4db,0x4a49034f,0x9d584a9d,
		0xd85c9a40,0xb1b371d6,0xdac74e64,0x9e4ef082,0xe6d33aab,0xf13d00ef,0x36db36ff,0xc961ae43,0x08e1be59,0xd9f7cda5,
		0x8d1aa05b,0xb6e08695,0x44837760,0x4039d1d9,0xfd99ce93,0x468bc977,0x678b55a8,0x4a8bfc65,0x5a56bc88,0xf95b932e,
		0x7f8276df,0x0b6a533a,0x71ad2701,0x4996c2c7,0xd6683196,0xc79db4c4,0xddd6394c,0xe5079e40,0x505d6bdf,0xeaaaf323,
		0x4107c14c,0xee9a766a,0xc5c89a73,0x3037697b,0x3a6ea2c2,0x5d3818bf,0xe85b091e,0xbe18b87e,0x2c403cf0,0x8bbf0003,
		0xea201d25,0x1c05605c,0xfb8a6a26,0xc6befae5,0x8c53236d,0x1fcf6ffc,0x5890100c,0xba848aae,0x0000e400,0x0380001c,
		0x5805d800,0xb8cdd0ee,0xec633371,0xe73c3191,0x8e989236,0x4bceb7ca,0x200419bd,0x00700004,0xc0000e00,0x00380001,
		0x147030b0,0xe000060c,0x001c0000,0x0f000380,0x24c2b1c4,0x78e8676a,0x01c7a420,0x982a7a56,0xf1efe784,0x0c5af054,
		0x03c2d620,0xb401cbcf,0x49005dea,0x80879020,0x56b0755a,0x9015c8e0,0xde53ed4a,0xb09f139f,0x9149b4f5,0x244d6df4,
		0xca95df2d,0xc45a5bc3,0x3d4b6458,0xec793408,0x87945438,0xa91ce7fb,0x94f78b39,0x5aec7b37,0xf28cea61,0xa8f359f0,
		0xd36afcea,0x240cf473,0x092b7d96,0xc9746d19,0xf8fc6742,0x331dced6,0xb3cdc90d,0xca3c0956,0xe625234e,0x63e9d955,
		0xa0cbddfb,0x7aa28d1e,0x4c110577,0x33fde970,0xeab93431,0x2b09767c,0x0a8db45b,0xf4a462d1,0x209de290,0xf28cf939,
		0x339d3770,0x8937bbfb,0xf1bf3f62,0x8cc95d49,0x879a525a,0xce55a883,0xe75be039,0xdeec1397,0xe4ab708a,0x0a1ac054,
		0xda8a7624,0xef32d7f7,0xd1990696,0x73ab29c9,0xae389b4b,0x7ae5a843,0x7a584b6e,0x38accce1,0x4667f0ea,0x53ad188b,
		0x96ddf4e8,0x6b8622d4,0x6093dad5,0xea883479,0x41e2c169,0x44bb56b9,0x3b2d00e7,0xb877fcb3,0xa976123c,0x64ccf20c,
		0x4c66f07f,0xf4f0b768,0x7c85d945,0x1c3f051c,0xad33b8c7,0xda218ff5,0xd86af651,0x7a1be25b,0x6fc538ee,0xfa99a6c7,
		0x4db45842,0x0723e41c,0x00435a00,0x32a03cf0,0xc0000c08,0xe2c78001,0xf93a170f,0x80553b95,0x00000090,0x4e454900,
		0x6042ae44,0x00000082,
	};

	scoped_allocation src((void*)TESTAsyncWrite, sizeof(TESTAsyncWrite), noop_deallocate);

	path TempFilePath = temp_path() / "TESTfilesystem_async_save.bin";
	remove(TempFilePath);
	event e;
	save_async(TempFilePath, std::move(src), [&](const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)
	{
		if (_pError)
			throw *_pError;
		e.set();
	});

	e.wait_for(std::chrono::seconds(10));
	filesystem::wait();

	scoped_allocation loaded = _Services.load_buffer(TempFilePath);
	oCHECK(memcmp(&TESTAsyncWrite, loaded, sizeof(TESTAsyncWrite)) == 0, "save_async failed to write correct file");
}

void TESTfilesystem(test_services& _Services)
{
	TESTfilesystem_paths();
	TESTfilesystem_read(_Services);
	TESTfilesystem_write();
	TESTfilesystem_map(_Services);
	TESTfilesystem_async1(_Services);
	TESTfilesystem_async2(_Services);
	TESTfilesystem_async_save(_Services);
}

	} // namespace tests
} // namespace ouro
