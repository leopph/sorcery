#pragma once

#include "leopphapi.h"
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
		std::vector<Mesh> m_Meshes;
		std::filesystem::path m_Directory;
		std::vector<Texture> m_CachedTextures;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType, Texture::TextureType abstractType);

	public:
		Model(const std::filesystem::path& path);

		Model(const Model& other) = default;
		Model(Model&& other) noexcept = default;

		Model& operator=(const Model& other) = default;
		Model& operator=(Model&& other) noexcept = default;

		bool operator==(const Model& other) const;

		void Draw(const Shader& shader) const;
	};

#pragma warning(pop)
}