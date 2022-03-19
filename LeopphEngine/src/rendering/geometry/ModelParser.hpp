#pragma once

#include "Mesh.hpp"
#include "../Texture.hpp"
#include "../../math/Matrix.hpp"

#include <assimp/scene.h>

#include <filesystem>
#include <memory>


namespace leopph::internal
{
	class ModelParser
	{
		public:
			[[nodiscard]]
			auto operator()(std::filesystem::path const& path) const -> std::vector<Mesh>;

		private:
			[[nodiscard]] static
			auto ProcessNodes(aiScene const* scene, std::filesystem::path const& path) -> std::vector<Mesh>;

			[[nodiscard]] static
			auto ProcessVertices(aiMesh const* mesh, Matrix4 const& trafo) -> std::vector<Vertex>;

			[[nodiscard]] static
			auto ProcessIndices(aiMesh const* mesh) -> std::vector<unsigned>;

			[[nodiscard]] static
			auto ProcessMaterial(aiScene const* scene, aiMesh const* mesh, std::filesystem::path const& path) -> std::shared_ptr<Material>;

			[[nodiscard]] static
			auto LoadTexture(aiMaterial const* material, aiTextureType type, std::filesystem::path const& path) -> std::shared_ptr<Texture>;

			[[nodiscard]] static
			auto ConvertTrafo(aiMatrix4x4 const& trafo) -> Matrix4;
	};
}
