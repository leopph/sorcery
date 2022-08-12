#include "Import.hpp"

#include "3d/ImportGeneric3d.hpp"
#include "3d/ImportLeopph3d.hpp"


namespace leopph
{
	std::vector<StaticMeshData> import_static_meshes(std::filesystem::path const& path)
	{
		if (path.extension() == ".leopph3d")
		{
			return import_static_leopph_3d(path);
		}

		return import_generic_static_meshes(path);
	}
}
