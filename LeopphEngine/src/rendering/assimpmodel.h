#pragma once

#include "mesh.h"

#include <assimp/scene.h>

#include <memory>

namespace leopph::impl
{
	class AssimpModelImpl
	{
	public:
		AssimpModelImpl(std::filesystem::path path);

		AssimpModelImpl(const AssimpModelImpl& other) = delete;

		AssimpModelImpl& operator=(const AssimpModelImpl& other) = delete;

		bool operator==(const AssimpModelImpl& other) const;

		void Draw(const Shader& shader) const;

		const std::filesystem::path& Path() const;

	private:
		std::vector<Mesh> m_Meshes;
		std::filesystem::path m_Directory;
		std::filesystem::path m_Path;

		void ProcessNode(aiNode* node, const aiScene* scene);
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
		std::unique_ptr<Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType);
	};
}