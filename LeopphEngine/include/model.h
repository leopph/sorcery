#pragma once

#include "leopph.h"
#include "mesh.h"

#include <assimp/scene.h>

namespace leopph
{
#pragma warning(push)
#pragma warning(disable: 4251)

	// CLASS TO REPRESENT A MODEL AS A COLLECTION OF MESHES
	class LEOPPHAPI Model
	{
	private:
		std::vector<Mesh> meshes;
		std::filesystem::path directory;

		std::vector<Texture> m_LoadedTextures;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* material, aiTextureType assimpType, Texture::TextureType abstractType);

	public:
		Model(const std::filesystem::path& path);

		Model(const Model& other) = delete;
		Model(Model&& other) noexcept;

		Model& operator=(const Model& other) = delete;
		Model& operator=(Model&& other) noexcept;

		void Draw(const Shader& shader) const;
	};

#pragma warning(pop)
}