#pragma once

#include "LeopphverterTypes.hpp"

#include <filesystem>
#include <optional>


namespace leopph::convert
{
	auto ProcessGenericModel(std::filesystem::path const& path) -> std::optional<Object>;
}
