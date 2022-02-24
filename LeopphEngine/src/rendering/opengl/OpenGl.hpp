#pragma once

#include <cstddef>
#include <vector>

namespace leopph::internal
{
	auto InitGL() -> bool;

	auto GlShaderBinaryFormats() -> std::vector<int>;
}
