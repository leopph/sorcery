#pragma once

#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	auto ParseLeopph3D(std::filesystem::path const& path) -> std::optional<Object>;
}
