#pragma once

#include "StaticModelData.hpp"

#include <filesystem>


namespace leopph
{
	StaticModelData import_static_meshes(std::filesystem::path const& path);
}
