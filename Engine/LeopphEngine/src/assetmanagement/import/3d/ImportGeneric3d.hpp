#pragma once

#include "../Import.hpp"

#include <filesystem>


namespace leopph
{
	std::vector<StaticMeshData> import_generic_static_meshes(std::filesystem::path const& path);
}
