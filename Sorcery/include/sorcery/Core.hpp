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


namespace sorcery {
static_assert(sizeof(float) == 4);
using f32 = float;

static_assert(sizeof(double) == 8);
using f64 = double;

template<typename T>
using NotNull = T;

template<typename T>
using MaybeNull = T;
}
