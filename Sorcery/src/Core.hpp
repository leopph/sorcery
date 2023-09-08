#pragma once

#ifndef _WIN64
#error "Only 64 bit Windows is supported."
#endif

#ifdef SORCERY_BUILD
#define LEOPPHAPI __declspec(dllexport)
#else
#define LEOPPHAPI __declspec(dllimport)
#endif


namespace sorcery {
static_assert(sizeof(float) == 4);
using f32 = float;

static_assert(sizeof(double) == 8);
using f64 = double;

template<typename T>
using ObserverPtr = T*;

template<typename T>
using NotNull = T;

template<typename T>
using MaybeNull = T;
}
