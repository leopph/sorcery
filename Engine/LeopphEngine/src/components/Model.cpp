#include "Model.hpp"

#include "Image.hpp"
#include "LeopphverterImport.hpp"
#include "Texture.hpp"

#include <utility>
#include <vector>


namespace leopph
{
	namespace
	{
		auto ConvertMaterial(convert::Material const& convMat, std::span<std::shared_ptr<Texture> const> const textures) -> std::shared_ptr<Material>
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


		auto Parse(std::filesystem::path const& path) -> std::vector<Mesh>
		{
			auto const& [convTexs, convMats, convMeshes] = convert::Import(path);

			std::vector<std::shared_ptr<Texture>> textures;
			textures.reserve(convTexs.size());

			for (auto const& convTex : convTexs)
			{
				textures.push_back(std::make_shared<Texture>(convTex));
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
		SwapRenderable(MeshGroup{Parse(m_Path)});
	}


	auto Model::Clone() const -> ComponentPtr<>
	{
		return CreateComponent<Model>(*this);
	}


	auto Model::Path() const noexcept -> std::filesystem::path const&
	{
		return m_Path;
	}
}
