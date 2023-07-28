#pragma once

#ifndef _WIN64
#error "Only 64 bit Windows is supported."
#endif

#ifdef SORCERY_BUILD
#define LEOPPHAPI __declspec(dllexport)
#else
#define LEOPPHAPI __declspec(dllimport)
#endif


#include <cstdint>


namespace sorcery {
using i8 = std::int8_t;
using u8 = std::uint8_t;
using i16 = std::int16_t;
using u16 = std::uint16_t;
using i32 = std::int32_t;
using u32 = std::uint32_t;
using i64 = std::int64_t;
using u64 = std::uint64_t;

// If building for Windows x86-64
#if defined(_MSC_FULL_VER) && defined(_M_X64)
using f32 = float;
using f64 = double;

constexpr auto CACHE_LINE_SIZE = 64;

template<typename T>
using ObserverPtr = T*;
#else
#error "Only Windows x86-64 is supported."
#endif
}
