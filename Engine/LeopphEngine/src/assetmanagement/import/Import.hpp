#pragma once

#include "StaticMeshData.hpp"

#include <filesystem>
#include <vector>


namespace leopph
{
	std::vector<StaticMeshData> import_static_meshes(std::filesystem::path const& path);
}
