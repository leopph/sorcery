#pragma once

#include "StaticModelData.hpp"
#include "LeopphApi.hpp"

#include <filesystem>


namespace leopph
{
	[[nodiscard]] LEOPPHAPI StaticModelData import_static_model(std::filesystem::path const& path);
}
