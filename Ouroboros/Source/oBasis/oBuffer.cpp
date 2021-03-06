// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oBuffer.h>
#include <oBase/assert.h>
#include <oString/fixed_string.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oLockedPointer.h>
#include <oBasis/oRefCount.h>
#include <oBasis/oURI.h>
#include <oConcurrency/mutex.h>

using namespace ouro;

struct oBuffer_Impl : public oBuffer
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oBuffer);

	oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, bool* _pSuccess);
	~oBuffer_Impl();

	void Lock() threadsafe { thread_cast<ouro::shared_mutex&>(RWMutex).lock();} \
	void LockRead() const threadsafe { thread_cast<ouro::shared_mutex&>(RWMutex).lock_shared(); } \
	void Unlock() threadsafe { thread_cast<ouro::shared_mutex&>(RWMutex).unlock(); } \
	void UnlockRead() const threadsafe { thread_cast<ouro::shared_mutex&>(RWMutex).unlock_shared(); } 

	void* GetData() override;
	const void* GetData() const override;

	size_t GetSize() const override;
	const char* GetName() const threadsafe override;

	void* Allocation;
	oInitOnce<uri_string> Name;
	size_t Size;
	DeallocateFn Deallocate;
	mutable ouro::shared_mutex RWMutex;
	oRefCount RefCount;
};

template<typename void_t, typename ptr_t>
bool oBufferCreateImpl(const char* _Name, void_t _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, ptr_t _ppBuffer)
{
	bool success = false;
	oCONSTRUCT(thread_cast<oBuffer**>(_ppBuffer), oBuffer_Impl(_Name, const_cast<void*>(_Allocation), _Size, _DeallocateFn, &success));  // casts are safe because the higher level oBufferCreate calls ensure thread and const safety
	return !!*_ppBuffer;
}

bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}
bool oBufferCreate(const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, const oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

bool oBufferCreate(const char* _Name, void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe oBuffer** _ppBuffer)
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

bool oBufferCreate( const char* _Name, const void* _Allocation, size_t _Size, oBuffer::DeallocateFn _DeallocateFn, threadsafe const oBuffer** _ppBuffer )
{
	return oBufferCreateImpl(_Name,  _Allocation, _Size, _DeallocateFn, _ppBuffer);
}

oBuffer_Impl::oBuffer_Impl(const char* _Name, void* _Allocation, size_t _Size, DeallocateFn _DeallocateFn, bool* _pSuccess)
	: Allocation(_Allocation)
	, Size(_Size)
	, Deallocate(_DeallocateFn)
	, Name(_Name)
{
	if (!Allocation || !Size)
	{
		oErrorSetLast(std::errc::invalid_argument);
		return;
	}

	*_pSuccess = true;
}

oBuffer_Impl::~oBuffer_Impl()
{
	if (Deallocate)
		Deallocate(Allocation);
}

void* oBuffer_Impl::GetData()
{
	return Allocation;
}

const void* oBuffer_Impl::GetData() const
{
	return Allocation;
}

size_t oBuffer_Impl::GetSize() const
{
	return Size;
}

const char* oBuffer_Impl::GetName() const threadsafe
{
	return Name->c_str();
}
