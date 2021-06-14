#pragma once

#include <assimp/scene.h>
#include "mesh.h"

namespace leopph::impl
{
	// CLASS TO REPRESENT A MODEL AS A COLLECTION OF MESHES
	class AssimpModelImpl
	{
	public:
		AssimpModelImpl(std::filesystem::path path);

		AssimpModelImpl(const AssimpModelImpl& other) = default;
		AssimpModelImpl(AssimpModelImpl&& other) noexcept = default;

		AssimpModelImpl& operator=(const AssimpModelImpl& other) = default;
		AssimpModelImpl& operator=(AssimpModelImpl&& other) noexcept = default;

		bool operator==(const AssimpModelImpl& other) const;

		void Draw(const Shader& shader) const;

		const std::filesystem::path& Path() const;

	private:
		std::vector<Mesh> m_Meshes;
		std::filesystem::path m_Directory;
		std::filesystem::path m_Path;
		std::vector<Texture> m_CachedTextures;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType, Texture::TextureType abstractType);
	};
}