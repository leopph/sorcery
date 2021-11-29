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
	class ModelImpl
	{
		public:
			explicit ModelImpl(std::filesystem::path path);
			ModelImpl(const ModelImpl& other) = delete;
			ModelImpl(ModelImpl&& other) = delete;

			~ModelImpl() = default;

			ModelImpl& operator=(const ModelImpl& other) = delete;
			ModelImpl& operator=(ModelImpl&& other) = delete;

			void DrawShaded(ShaderProgram& shader, const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices, std::size_t nextFreeTextureUnit) const;
			void DrawDepth(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const;

			[[nodiscard]] bool CastsShadow() const;
			void CastsShadow(bool value);

			const std::filesystem::path Path;


		private:
			std::vector<Mesh> m_Meshes;
			std::filesystem::path m_Directory;

			void ProcessNodes(const aiScene* scene);
			Mesh ProcessMesh(const aiMesh* mesh, const aiScene* scene, const Matrix3& trafo) const;
			std::optional<Texture> LoadTexturesByType(const aiMaterial* material, aiTextureType assimpType) const;
			void AdjustInstanceBuffer(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const;

			mutable std::size_t m_InstanceBufferSize;
			unsigned m_InstanceBuffer;

			bool m_CastsShadow;
	};
}
