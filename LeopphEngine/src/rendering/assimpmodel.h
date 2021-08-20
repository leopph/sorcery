#pragma once

#include "mesh.h"
#include "../math/Matrix.hpp"

#include <assimp/scene.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace leopph::impl
{
	class AssimpModelImpl
	{
	public:
		AssimpModelImpl(std::filesystem::path path);

		AssimpModelImpl(const AssimpModelImpl& other) = delete;

		AssimpModelImpl& operator=(const AssimpModelImpl& other) = delete;

		bool operator==(const AssimpModelImpl& other) const;

		void DrawShaded(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices) const;
		void DrawDepth(const Shader& shader, const std::vector<Matrix4>& modelMatrices) const;

		const std::filesystem::path& Path() const;

		void OnReferringObjectsChanged(std::size_t newAmount) const;

	private:
		std::vector<Mesh> m_Meshes;
		std::filesystem::path m_Directory;
		std::filesystem::path m_Path;

		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const Matrix3& trafo);
		std::unique_ptr<Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType);
	};
}