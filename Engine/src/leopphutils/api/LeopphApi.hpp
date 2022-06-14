#pragma once

#ifndef _WIN64
#error "Only 64 bit Windows is supported."
#endif

#ifdef ENGINE_BUILD
#define LEOPPHAPI __declspec(dllexport)
#else
	#define LEOPPHAPI __declspec(dllimport)
#endif
