#pragma once

#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	std::optional<Object> ProcessGenericModel(std::filesystem::path const& path);
}
