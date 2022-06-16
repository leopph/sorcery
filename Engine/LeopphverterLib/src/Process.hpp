#pragma once

#include "LeopphverterCommon.hpp"

#include <filesystem>


namespace leopph::convert
{
	auto ProcessGenericModel(std::filesystem::path const& path) -> Object;
}
