#pragma once

#include "ModelData.hpp"
#include "../Texture.hpp"
#include "../../math/Matrix.hpp"

#include <assimp/scene.h>

#include <filesystem>
#include <optional>


namespace leopph::impl
{
	class FileModelData : public ModelData
	{
	public:
		FileModelData(std::filesystem::path path);

		const std::filesystem::path Path;


	private:
		std::vector<impl::MeshData> m_MeshData;

		std::vector<impl::MeshData> ProcessNodes(const aiScene* scene) const;
		impl::MeshData ProcessMesh(const aiMesh* mesh, const aiScene* scene, const Matrix3& trafo) const;
		std::optional<Texture> LoadTexture(const aiMaterial* material, const aiTextureType type) const;
	};
}