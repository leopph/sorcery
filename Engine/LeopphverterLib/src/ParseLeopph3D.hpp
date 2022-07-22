#pragma once

#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	std::optional<Object> ParseLeopph3D(std::filesystem::path const& path);
}
