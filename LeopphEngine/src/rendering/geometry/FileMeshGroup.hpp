#pragma once

#include "MeshGroup.hpp"
#include "../Texture.hpp"
#include "../../math/Matrix.hpp"

#include <assimp/scene.h>

#include <filesystem>
#include <memory>


namespace leopph::internal
{
	class FileMeshGroup final : public MeshGroup
	{
		public:
			explicit FileMeshGroup(std::filesystem::path path);

			[[nodiscard]]
			auto Path() const -> const std::filesystem::path&;

		private:
			std::filesystem::path m_Path;

			auto ProcessNodes(const aiScene* scene) const -> std::vector<Mesh>;
			auto ProcessMesh(const aiMesh* mesh, const aiScene* scene, const Matrix3& trafo) const -> Mesh;
			auto LoadTexture(const aiMaterial* material, aiTextureType type) const -> std::shared_ptr<Texture>;
	};
}
