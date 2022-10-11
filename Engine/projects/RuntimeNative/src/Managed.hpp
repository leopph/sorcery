#pragma once

#include "Core.hpp"

#include <vector>


struct _MonoMethod;
struct _MonoObject;
struct _MonoClass;

typedef _MonoMethod MonoMethod;
typedef _MonoObject MonoObject;
typedef _MonoClass MonoClass;


namespace leopph
{
	LEOPPHAPI void initialize_managed_runtime();
	LEOPPHAPI void cleanup_managed_runtime();
}