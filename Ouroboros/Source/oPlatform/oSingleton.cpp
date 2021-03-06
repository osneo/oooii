// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oPlatform/oSingleton.h>
#include <oCore/process_heap.h>
#include <oConcurrency/backoff.h>

using namespace ouro;

// @tony: Singletons are useful concepts and really cross-platform. It is
// unfortunate that on at least Windows it requires important platform API calls
// to enforce true singleness across DLL boundaries. Because it would be really
// nice to promote this to oBasis, encapsulate all platform calls here in this
// namespace

//#define oENABLE_SINGLETON_TRACE

namespace oSingletonPlatform
{
#ifdef oENABLE_SINGLETON_TRACE
	// use our own trace because assert and debugger are based on singletons, so 
	// they might be gone or themselves being destructed when we want to print out
	// debug info.
	static void Trace(const char* _TypeinfoName, const char* _File, int _Line, const char* _Format, ...)
	{
		path modname = ouro::this_module::path();
		char syspath[_MAX_PATH];
		char msg[oKB(4)];
		int offset = snprintf(msg, "%s(%d): {%s} %s %s ", _File, _Line, modname.basename().c_str(), oSystemGetExecutionPath(syspath), type_name(_TypeinfoName));
		va_list args;
		va_start(args, _Format);
		vsnprintf(msg + offset, oCOUNTOF(msg) - offset, _Format, args);
		va_end(args);
		oStrcat(msg, "\n");
		ouro::debugger::print(msg);
	}
#endif

	ouro::module::id GetCurrentModule() { return ouro::this_module::get_id(); }

} // namespace oSingletonPlatform

#ifdef oENABLE_SINGLETON_TRACE
	#define oSINGLETON_TRACE(_Name, format, ...) do { oSingletonPlatform::Trace(_Name, __FILE__, __LINE__, format, ## __VA_ARGS__); } while(false)
#else
	#define oSINGLETON_TRACE(_Name, format, ...) __noop
#endif

bool oConstructOnceV(std::atomic<void*>* _pPointer, void* (*_New)())
{
	static void* CONSTRUCTING = (void*)0x1;
	bool constructed = false;
	if (*_pPointer <= CONSTRUCTING)
	{
		void* pNull = nullptr;
		if (_pPointer->compare_exchange_strong(pNull, CONSTRUCTING))
		{
			void* p = _New();
			*_pPointer = p;
			constructed = true;
		}

		else
		{
			backoff bo;
			while (*_pPointer <= CONSTRUCTING)
				bo.pause();
		}
	}

	return constructed;
}
	
// {7F15EF8E-3AA2-43D8-B802-06A3E195E21C}
static const oGUID IID_oSingletonCtors = { 0x7f15ef8e, 0x3aa2, 0x43d8, { 0xb8, 0x2, 0x6, 0xa3, 0xe1, 0x95, 0xe2, 0x1c } };
typedef std::unordered_map<oGUID, type_info_default_constructor, std::hash<oGUID>, std::equal_to<oGUID>, process_heap::std_allocator<std::pair<oGUID, type_info_default_constructor>>> oSingletonCtors;
void oPlacementNewSingletonCtor(void* _Pointer) { new(_Pointer) oSingletonCtors(); }

void* oSingletonCtorRegistryCreate()
{
	void* p = nullptr;

	sstring StrGUID;
	process_heap::find_or_allocate(sizeof(oSingletonCtors)
		, to_string(StrGUID, IID_oSingletonCtors)
		, process_heap::per_process
		, process_heap::none
		, oPlacementNewSingletonCtor
		, nullptr
		, &p);
	
	return p;
}

oSingletonRegister::oSingletonRegister(const char* _SingletonName, const oGUID& _SingletonGUID, type_info_default_constructor _PlacementNew)
{
	oSingletonCtors& ctors = *(oSingletonCtors*)oSingletonCtorRegistryCreate();
	if (ctors.find(_SingletonGUID) == ctors.end())
	{
		ctors[_SingletonGUID] = _PlacementNew;
		oSINGLETON_TRACE(_SingletonName, "registered");
	}
}

oSingletonBase::oSingletonBase(int _InitialRefCount)
	: Name("")
	, RefCount(_InitialRefCount)
{
	auto id = oSingletonPlatform::GetCurrentModule();
	hModule = *(void**)&id;
}

oSingletonBase::~oSingletonBase()
{
	oSINGLETON_TRACE(Name, "deinitialized");
}

int oSingletonBase::Reference() threadsafe
{
	int r = RefCount.Reference();
	oSINGLETON_TRACE(typeid(*this).name(), "referenced %d -> %d", r-1, r);
	return r;
}

void oSingletonBase::Release() threadsafe
{
	int r = RefCount;
	oSINGLETON_TRACE(typeid(*this).name(), "released %d -> %d", r, r-1);
	if (RefCount.Release())
	{
		#if oENABLE_ASSERTS
			auto id = oSingletonPlatform::GetCurrentModule();
			oASSERT(hModule == *(void**)&id, "Singleton being freed by a module different than the one creating it.");
		#endif
		this->~oSingletonBase();
		process_heap::deallocate(thread_cast<oSingletonBase*>(this)); // safe because we're not using this anymore
	}
}
	
void* oSingletonBase::NewV(const char* _TypeInfoName, size_t _Size, type_info_default_constructor _Ctor, const oGUID& _GUID, bool _IsThreadLocal)
{
	type_info_default_constructor PlacementNew = _Ctor;

	oSingletonCtors* ctors = nullptr;
	sstring StrGUID;
	if (process_heap::find(to_string(StrGUID, IID_oSingletonCtors), process_heap::per_process, (void**)&ctors))
	{
		auto it = ctors->find(_GUID);
		oASSERT(it != ctors->end(), "%s ctor not found (did you use forget oSINGLETON_REGISTER?)", _TypeInfoName);
		PlacementNew = it->second;
	}

	oSingletonBase* p = nullptr;
	if (process_heap::find_or_allocate(
		_Size
		, to_string(StrGUID, _GUID)
		, _IsThreadLocal ? process_heap::per_thread : process_heap::per_process
		, process_heap::leak_tracked
		, PlacementNew
		, nullptr
		, (void**)&p))
	{
		p->Name = type_name(_TypeInfoName);
		oSINGLETON_TRACE(_TypeInfoName, "%ssingleton initialized using %s ctor", _IsThreadLocal ? "threadlocal " : "", !ctors ? "module-local" : "prime-module");

		if (_IsThreadLocal)
		{
			oASSERT(false, "deprecated");
			//oThreadlocalRegistry::Singleton()->RegisterThreadlocalSingleton(p);
			p->Release();
		}
	}

	else if (!_IsThreadLocal)
		p->Reference();

	return p;
}
