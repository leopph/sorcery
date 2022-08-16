#include "Import.hpp"

#include "Logger.hpp"
#include "../../rendering/StaticMaterial.hpp"
#include "../../rendering/StaticMesh.hpp"
#include "../../rendering/Texture2D.hpp"
#include "3d/ImportGeneric3d.hpp"
#include "3d/ImportLeopph3d.hpp"


namespace leopph
{
	StaticModelData import_static_model(std::filesystem::path const& path)
	{
		if (path.extension() == ".leopph3d")
		{
			// TODO fix leopph3d
			//return import_static_leopph_3d(path);
			Logger::get_instance().trace("Skipping leopph3d file because the format is temporarily disabled.");
			return {};
		}

		return import_generic_static_model(path);
	}



	std::vector<std::pair<std::shared_ptr<StaticMesh>, std::shared_ptr<StaticMaterial>>> generate_render_structures(StaticModelData const& modelData)
	{
		std::vector<std::shared_ptr<Texture2D const>> textures;
		textures.reserve(modelData.textures.size());

		for (auto const& image : modelData.textures)
		{
			textures.emplace_back(std::make_shared<Texture2D>(image));
		}

		std::vector<std::shared_ptr<StaticMaterial>> materials;
		materials.reserve(modelData.materials.size());

		for (auto const& materialData : modelData.materials)
		{
			materials.emplace_back(std::make_shared<StaticMaterial>(materialData, textures));
		}

		std::vector<std::shared_ptr<StaticMesh>> meshes;
		meshes.reserve(modelData.meshes.size());

		for (auto const& meshData : modelData.meshes)
		{
			meshes.emplace_back(std::make_shared<StaticMesh>(meshData));
		}

		std::vector<std::pair<std::shared_ptr<StaticMesh>, std::shared_ptr<StaticMaterial>>> ret;
		ret.reserve(meshes.size() > materials.size() ? materials.size() : meshes.size());

		for (std::size_t i = 0; i < materials.size() && i < meshes.size(); i++)
		{
			ret.emplace_back(std::move(meshes[i]), std::move(materials[i]));
		}

		return ret;
	}
}
