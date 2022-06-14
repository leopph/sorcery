#pragma once

#include "LeopphverterCommon.hpp"

#include <filesystem>


namespace leopph::convert
{
	auto Import(std::filesystem::path const& path) -> Object;
}
