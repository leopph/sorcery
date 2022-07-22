#pragma once

#include "Leopphverter.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	std::optional<Object> decode_generic_3d_asset(std::filesystem::path const& path);
}
