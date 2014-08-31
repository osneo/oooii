// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Forward declaration of compiler intrinsic functions. This is declared here
// instead of using <intrin.h> because of conflicts in MSVC between declarations
// of ceil in <intrin.h> and <math.h>
#pragma once
#ifndef oHLSLIntrinsics_h
#define oHLSLIntrinsics_h

#ifdef __cplusplus
	extern "C" {
#endif

// _____________________________________________________________________________
// Bit scanning

unsigned char _BitScanForward(unsigned long* Index, unsigned long Mask);
unsigned char _BitScanReverse(unsigned long* Index, unsigned long Mask);
#pragma intrinsic(_BitScanReverse)
#pragma intrinsic(_BitScanForward)

#ifdef _M_X64
	unsigned char _BitScanForward64(unsigned long* Index, unsigned __int64 Mask);
	unsigned char _BitScanReverse64(unsigned long* Index, unsigned __int64 Mask);
	#pragma intrinsic(_BitScanReverse64)
	#pragma intrinsic(_BitScanForward64)
#endif

// _____________________________________________________________________________
// Atomics

void _ReadBarrier();
void _ReadWriteBarrier();

long _InterlockedAdd(long volatile *Addend, long Value);
short _InterlockedAnd16(short volatile *Destination, short Value);
long _InterlockedAnd(long volatile *Destination, long Value);
short _InterlockedIncrement16(short volatile *Addend);
long _InterlockedIncrement(long volatile *Addend);
long long _InterlockedIncrement64(long long volatile *Addend);
short _InterlockedDecrement16(short volatile *Addend);
long _InterlockedDecrement(long volatile *Addend);
long long _InterlockedDecrement64(long long volatile *Addend);
char _InterlockedExchange8(char volatile *Target, char Value);
short _InterlockedExchange16(short volatile *Target, short Value);
long _InterlockedExchange(long volatile *Target, long Value);
long _InterlockedExchangeAdd(long volatile *Addend, long Value);
char  _InterlockedCompareExchange8(char  volatile *Destination, char  ExChange, char  Comperand);
short _InterlockedCompareExchange16(short volatile *Destination, short ExChange, short Comperand);
long _InterlockedCompareExchange(long volatile *Destination, long ExChange, long Comperand);
short _InterlockedOr16(short volatile *Destination, short Value);
long _InterlockedOr(long volatile *Destination, long Value);
short _InterlockedXor16(short volatile *Destination, short Value);
long _InterlockedXor(long volatile *Destination, long Value);

//#pragma intrinsic(_InterlockedAdd)
#pragma intrinsic(_InterlockedAnd16)
#pragma intrinsic(_InterlockedAnd)
#pragma intrinsic(_InterlockedIncrement16)
#pragma intrinsic(_InterlockedIncrement)
#pragma intrinsic(_InterlockedDecrement16)
#pragma intrinsic(_InterlockedDecrement)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedOr16)
#pragma intrinsic(_InterlockedOr)
#pragma intrinsic(_InterlockedXor16)
#pragma intrinsic(_InterlockedXor)

#if _MSC_VER >= 1600
	#pragma intrinsic(_InterlockedExchange8)
	#pragma intrinsic(_InterlockedCompareExchange8)
	#pragma intrinsic(_InterlockedExchange16)
	#pragma intrinsic(_InterlockedCompareExchange16)
#endif

long long _InterlockedAdd64(long long volatile *Addend, long long Value);
long long _InterlockedAnd64(long long volatile *Destination, long long Value);
long long _InterlockedIncrement64(long long volatile *Addend);
long long _InterlockedDecrement64(long long volatile *Addend);
long long _InterlockedExchange64(long long volatile *Target, long long Value);
long long _InterlockedCompareExchange64(long long volatile *Destination, long long ExChange, long long Comperand);
long long _InterlockedOr64(long long volatile *Destination, long long Value);
long long _InterlockedXor64(long long volatile *Destination, long long Value);
#ifdef _M_X64
	void* _InterlockedExchangePointer(void* volatile *Target, void* Value);
	void* _InterlockedCompareExchangePointer(void* volatile *Destination, void* Exchange, void* Comperand);
	#pragma intrinsic(_InterlockedExchangePointer)
	#pragma intrinsic(_InterlockedCompareExchangePointer)
	//#pragma intrinsic(_InterlockedAdd64)
	#pragma intrinsic(_InterlockedAnd64)
	#pragma intrinsic(_InterlockedIncrement64)
	#pragma intrinsic(_InterlockedDecrement64)
	#pragma intrinsic(_InterlockedExchange64)
	#pragma intrinsic(_InterlockedCompareExchange64)
	#pragma intrinsic(_InterlockedOr64)
#pragma intrinsic(_InterlockedXor64)
#endif

#ifdef __cplusplus
	}
#endif
#endif
