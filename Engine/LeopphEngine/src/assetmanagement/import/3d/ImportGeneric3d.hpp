#pragma once

#include "../Import.hpp"

#include <filesystem>


namespace leopph
{
	StaticModelData import_generic_static_model(std::filesystem::path const& path);
}
