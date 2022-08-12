#include "Import.hpp"

#include "Logger.hpp"
#include "3d/ImportGeneric3d.hpp"
#include "3d/ImportLeopph3d.hpp"


namespace leopph
{
	StaticModelData import_static_meshes(std::filesystem::path const& path)
	{
		if (path.extension() == ".leopph3d")
		{
			// TODO fix leopph3d
			//return import_static_leopph_3d(path);
			internal::Logger::Instance().Trace("Skipping leopph3d file because the format is temporarily disabled.");
			return {};
		}

		return import_generic_static_meshes(path);
	}
}
