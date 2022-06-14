#pragma once

#include "Image.hpp"
#include "Matrix.hpp"
#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/mesh.h>

#include <filesystem>
#include <memory>
#include <vector>


namespace leopph::internal
{
	class ModelParser
	{
		public:
			[[nodiscard]]
			auto Parse(std::filesystem::path path) -> std::vector<Mesh>;

		private:
			[[nodiscard]]
			auto ProcessNodes() -> std::vector<Mesh>;

			[[nodiscard]] static
			auto ProcessVertices(aiMesh const* mesh, Matrix4 const& trafo) -> std::vector<Vertex>;

			[[nodiscard]] static
			auto ProcessIndices(aiMesh const* mesh) -> std::vector<unsigned>;

			[[nodiscard]]
			auto ProcessMaterial(aiMaterial const* assimpMat) const -> std::shared_ptr<Material>;

			[[nodiscard]]
			auto LoadTextureImage(aiString const& texPath) const -> Image;

			[[nodiscard]] static
			auto ConvertTrafo(aiMatrix4x4 const& trafo) -> Matrix4;

			Assimp::Importer m_Importer;
			// Stores the materials in the same order as aiScene::mMaterials
			std::vector<std::shared_ptr<Material>> m_Materials;
			std::filesystem::path m_Path;
	};
}
