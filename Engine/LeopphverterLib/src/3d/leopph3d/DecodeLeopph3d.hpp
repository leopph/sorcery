#pragma once

#include "Leopphverter.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	std::optional<Object> decode_leopph3d(std::filesystem::path const& path);
}
