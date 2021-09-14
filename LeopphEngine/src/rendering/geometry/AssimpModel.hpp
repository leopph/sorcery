#pragma once

#include "../../math/Matrix.hpp"
#include "../shaders/Shader.hpp"
#include "../Texture.hpp"
#include "Mesh.hpp"

#include <assimp/scene.h>

#include <cstddef>
#include <filesystem>
#include <memory>
#include <vector>


namespace leopph::impl
{
	class AssimpModel
	{
	public:
		AssimpModel(const std::filesystem::path& path);

		AssimpModel(const AssimpModel& other) = delete;
		AssimpModel(AssimpModel&& other) = delete;
		AssimpModel& operator=(const AssimpModel& other) = delete;
		AssimpModel& operator=(AssimpModel&& other) = delete;

		~AssimpModel() = default;

		void DrawShaded(const Shader& shader, const std::vector<Matrix4>& modelMatrices, const std::vector<Matrix4>& normalMatrices, std::size_t nextFreeTextureUnit) const;
		void DrawDepth(const std::vector<Matrix4>& modelMatrices) const;
		void OnReferringEntitiesChanged(std::size_t newAmount) const;


	private:
		std::vector<Mesh> m_Meshes;
		std::filesystem::path m_Directory;

		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, const Matrix3& trafo);
		std::unique_ptr<Texture> LoadTexturesByType(aiMaterial* material, aiTextureType assimpType);
	};
}