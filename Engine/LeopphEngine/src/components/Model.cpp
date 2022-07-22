#include "Model.hpp"

#include "GlTexture.hpp"
#include "Image.hpp"
#include "LeopphverterImport.hpp"

#include <utility>
#include <vector>


namespace leopph
{
	namespace
	{
		std::shared_ptr<Material> ConvertMaterial(convert::Material const& convMat, std::span<std::shared_ptr<GlTexture> const> const textures)
		{
			auto mat = std::make_shared<Material>();

			mat->DiffuseColor = convMat.DiffuseColor;
			mat->SpecularColor = convMat.SpecularColor;
			mat->Gloss = convMat.Gloss;
			mat->TwoSided = convMat.TwoSided;
			mat->Opacity = convMat.Opacity;

			if (convMat.DiffuseMap.has_value())
			{
				mat->DiffuseMap = textures[convMat.DiffuseMap.value()];
			}

			if (convMat.SpecularMap.has_value())
			{
				mat->SpecularMap = textures[convMat.SpecularMap.value()];
			}

			if (convMat.OpacityMap.has_value())
			{
				mat->OpacityMap = textures[convMat.OpacityMap.value()];
			}

			return mat;
		}


		std::vector<Mesh> Parse(std::filesystem::path const& path)
		{
			auto const imported = convert::Import(path);

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
				mats.push_back(ConvertMaterial(convMat, textures));
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
		Init(MeshGroup{Parse(m_Path)});
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
