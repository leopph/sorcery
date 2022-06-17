#pragma once

#include "LeopphApi.hpp"
#include "LeopphverterTypes.hpp"

#include <filesystem>


namespace leopph::convert
{
	LEOPPHAPI auto Import(std::filesystem::path const& path) -> Object;
}
