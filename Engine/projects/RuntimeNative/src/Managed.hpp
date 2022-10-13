#pragma once

#include "Core.hpp"

namespace leopph
{
	LEOPPHAPI [[nodiscard]] bool initialize_managed_runtime();
	LEOPPHAPI void cleanup_managed_runtime();
}