#pragma once

#include "Core.hpp"

using MonoImage = struct _MonoImage;
using MonoDomain = struct _MonoDomain;

namespace leopph
{
	LEOPPHAPI [[nodiscard]] bool initialize_managed_runtime();
	LEOPPHAPI void cleanup_managed_runtime();	
	[[nodiscard]] MonoImage* GetManagedImage();
	[[nodiscard]] MonoDomain* GetManagedDomain();
}