#include "Model.hpp"

#include "GlTexture.hpp"
#include "Image.hpp"
#include "Leopphverter.hpp"

#include <utility>
#include <vector>


namespace leopph
{
	namespace
	{
		std::shared_ptr<Material> convert_material(convert::Material const& convMat, std::span<std::shared_ptr<GlTexture> const> const textures)
		{
			auto mat = std::make_shared<Material>();

			mat->DiffuseColor = convMat.diffuseColor;
			mat->SpecularColor = convMat.specularColor;
			mat->Gloss = convMat.gloss;
			mat->TwoSided = convMat.twoSided;
			mat->Opacity = convMat.opacity;

			if (convMat.diffuseMap)
			{
				mat->DiffuseMap = textures[*convMat.diffuseMap];
			}

			if (convMat.specularMap)
			{
				mat->SpecularMap = textures[*convMat.specularMap];
			}

			if (convMat.opacityMap)
			{
				mat->OpacityMap = textures[*convMat.opacityMap];
			}

			return mat;
		}



		std::vector<Mesh> parse(std::filesystem::path const& path)
		{
			auto const imported = convert::import_3d_asset(path);

			if (!imported.has_value())
			{
				return {};
			}

			auto const& [convTexs, convMats, convMeshes] = imported.value();

			std::vector<std::shared_ptr<GlTexture>> textures;
			textures.reserve(convTexs.size());

			for (auto const& convTex : convTexs)
			{
				textures.push_back(std::make_shared<GlTexture>(convTex));
			}

			std::vector<std::shared_ptr<Material>> mats;
			mats.reserve(convMats.size());

			for (auto const& convMat : convMats)
			{
				mats.push_back(convert_material(convMat, textures));
			}

			std::vector<Mesh> meshes;
			meshes.reserve(convMeshes.size());

			for (auto const& [vertices, indices, matIndex] : convMeshes)
			{
				meshes.emplace_back(vertices, indices, mats[matIndex]);
			}

			return meshes;
		}
	}



	Model::Model(std::filesystem::path path) :
		m_Path{std::move(path)}
	{
		Init(MeshGroup{parse(m_Path)});
	}



	ComponentPtr<> Model::Clone() const
	{
		return CreateComponent<Model>(*this);
	}



	std::filesystem::path const& Model::Path() const noexcept
	{
		return m_Path;
	}
}
