#pragma once

#ifndef _WIN64
#error "Only 64 bit Windows is supported."
#endif

#ifdef SORCERY_BUILD
#define SORCERYAPI __declspec(dllexport)
#else
#define SORCERYAPI __declspec(dllimport)
#endif

#define LEOPPHAPI SORCERYAPI

#include <cstddef>


namespace sorcery {
// Fixed instead of std::hardware_destructive_interference_size so that layouts don't depend on compiler flags
// across the DLL boundary. 64 bytes is correct for every supported (x64) target.
constexpr std::size_t kCacheLineSize{64};

static_assert(sizeof(float) == 4);
using f32 = float;

static_assert(sizeof(double) == 8);
using f64 = double;

template<typename T>
using NotNull = T;

template<typename T>
using MaybeNull = T;
}
