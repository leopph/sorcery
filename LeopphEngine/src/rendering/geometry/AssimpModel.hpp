#pragma once

#include "Mesh.hpp"
#include "../Texture.hpp"
#include "../../math/Matrix.hpp"
#include "../shaders/ShaderProgram.hpp"

#include <assimp/scene.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <optional>
#include <vector>



namespace leopph::impl
{
	class AssimpModel
	{
		public:
			explicit AssimpModel(const std::filesystem::path& path);

			AssimpModel(const AssimpModel& other) = delete;
			AssimpModel(AssimpModel&& other) = delete;
			AssimpModel& operator=(const AssimpModel& other) = delete;
			AssimpModel& operator=(AssimpModel&& other) = delete;

			~AssimpModel() = default;

			void DrawShaded(ShaderProgram& shader, const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices, std::size_t nextFreeTextureUnit);
			void DrawDepth(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices);


		private:
			std::vector<Mesh> m_Meshes;
			std::filesystem::path m_Directory;

			void ProcessNodes(const aiScene* scene);
			Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene, const Matrix3& trafo) const;
			std::optional<Texture> LoadTexturesByType(const aiMaterial* material, aiTextureType assimpType) const;
			void AdjustInstanceBuffer(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices);

			std::size_t m_InstanceBufferSize;
			unsigned m_InstanceBuffer;
	};
}
