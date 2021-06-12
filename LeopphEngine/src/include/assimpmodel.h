#pragma once

#include <assimp/scene.h>
#include "mesh.h"

namespace leopph::impl
{
	// CLASS TO REPRESENT A MODEL AS A COLLECTION OF MESHES
	class AssimpModelImpl
	{
	private:
		std::vector<impl::Mesh> m_Meshes;
		std::filesystem::path m_Directory;
		std::vector<impl::Texture> m_CachedTextures;

		void ProcessNode(aiNode* node, const aiScene* scene);
		impl::Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<impl::Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType, impl::Texture::TextureType abstractType);

	public:
		AssimpModelImpl(const std::filesystem::path& path);

		AssimpModelImpl(const AssimpModelImpl& other) = default;
		AssimpModelImpl(AssimpModelImpl&& other) noexcept = default;

		AssimpModelImpl& operator=(const AssimpModelImpl& other) = default;
		AssimpModelImpl& operator=(AssimpModelImpl&& other) noexcept = default;

		bool operator==(const AssimpModelImpl& other) const;

		void Draw(const impl::Shader& shader) const;
	};
}