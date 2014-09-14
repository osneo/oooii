// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Utilities helpful when dealing with little v. big endian data serialization.

#pragma once
#include <stddef.h>
#include <cstdint>
#include <type_traits>

namespace ouro { namespace detail { static const uint16_t endian_signature = 1; }

static const bool is_little_endian = *(const uint8_t*)&detail::endian_signature == 1;
static const bool is_big_endian = *(const uint8_t*)&detail::endian_signature == 0;

template<typename T> T endian_swap(const T& x) { static_assert(!std::is_floating_point<T>::value, "floating point values must use explicit api for endian swapping"); return x; }
template<> inline uint16_t endian_swap(const uint16_t& x) { return (x<<8) | (x>>8); }
template<> inline uint32_t endian_swap(const uint32_t& x) { return (x<<24) | ((x<<8) & 0x00ff0000) | ((x>>8) & 0x0000ff00) | (x>>24); }
template<> inline uint64_t endian_swap(const uint64_t& x) { return (x<<56) | ((x<<40) & 0x00ff000000000000ll) | ((x<<24) & 0x0000ff0000000000ll) | ((x<<8) & 0x000000ff00000000ll) | ((x>>8) & 0x00000000ff000000ll) | ((x>>24) & 0x0000000000ff0000ll) | ((x>>40) & 0x000000000000ff00ll) | (x>>56); }
template<> inline int16_t endian_swap(const int16_t& x) { uint16_t r = endian_swap(*(uint16_t*)&x); return *(int16_t*)&r; }
template<> inline int32_t endian_swap(const int32_t& x) { uint32_t r = endian_swap(*(uint32_t*)&x); return *(int32_t*)&r; }
template<> inline int64_t endian_swap(const int64_t& x) { uint64_t r = endian_swap(*(uint64_t*)&x); return *(int64_t*)&r; }

inline uint32_t endian_swap_float(const float& x) { return endian_swap(*(uint32_t*)&x); }
inline float endian_swap_float(const uint32_t& x) { uint32_t f = endian_swap(x); return *(float*)&f; }

inline uint64_t endian_swap_double(const double& x) { return endian_swap(*(uint64_t*)&x); }
inline double endian_swap_double(const uint64_t& x) { uint64_t f = endian_swap(x); return *(double*)&f; }

template<typename T> T to_big_endian(const T& x) { return is_little_endian ? endian_swap(x) : x; };
template<typename T> T from_big_endian(const T& x) { return to_big_endian(x); }
template<typename T> T to_little_endian(T x) { return is_little_endian ? x : endian_swap(x); };
template<typename T> T from_little_endian(T x) { return to_little_endian(x); }

inline uint32_t to_big_endian_float(const float& x) { return is_little_endian ? endian_swap_float(x) : *(uint32_t*)&x; };
inline float to_big_endian_float(const uint32_t& x) { return is_little_endian ? endian_swap_float(x) : *(float*)&x; };
inline uint32_t from_big_endian_float(const float& x) { return to_big_endian_float(x); }
inline float from_big_endian_float(const uint32_t& x) { return to_big_endian_float(x); }

inline uint64_t to_big_endian_double(const double& x) { return is_little_endian ? endian_swap_double(x) : *(uint64_t*)&x; };
inline double to_big_endian_double(const uint64_t& x) { return is_little_endian ? endian_swap_double(x) : *(double*)&x; };
inline uint64_t from_big_endian_double(const double& x) { return to_big_endian_double(x); }
inline double from_big_endian_double(const uint64_t& x) { return to_big_endian_double(x); }

}
