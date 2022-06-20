#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	LEOPPHAPI auto Import(std::filesystem::path const& path) -> std::optional<Object>;
}
