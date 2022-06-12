#include "Import.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>


namespace leopph::convert
{
	namespace
	{
		Material ProcessMaterial(aiMaterial const* aiMat)
		{
			
		}
	}

	std::vector<Mesh> Import(std::filesystem::path const& path)
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "Error parsing model from file at " << path << ": " << importer.GetErrorString() << '\n';
			return {};
		}

		std::vector<Material> materials;
		for (unsigned i = 0; i < scene->mNumMaterials; i++)
		{
			materials.push_back(ProcessMaterial(scene->mMaterials[i]));
		}
	}

}