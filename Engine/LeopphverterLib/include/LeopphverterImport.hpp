#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	LEOPPHAPI std::optional<Object> Import(std::filesystem::path const& path);
}
