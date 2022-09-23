#pragma once

#ifdef EXPORT
#define LEOPPHAPI __declspec(dllexport)
#else
#define LEOPPHAPI __declspec(dllimport)
#endif