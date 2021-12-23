#pragma once

#ifdef ENGINE_BUILD
#define LEOPPHAPI __declspec(dllexport)
#else
	#define LEOPPHAPI __declspec(dllimport)
#endif
