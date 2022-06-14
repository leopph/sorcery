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
		auto ConvertMaterial(convert::Material const& convMat, std::filesystem::path const& rootPath) -> std::shared_ptr<Material>
		{
			auto mat = std::make_shared<Material>();

			mat->DiffuseColor = convMat.DiffuseColor;
			mat->SpecularColor = convMat.SpecularColor;
			mat->Gloss = convMat.Gloss;
			mat->TwoSided = convMat.TwoSided;
			mat->Opacity = convMat.Opacity;

			if (!convMat.OpacityMap.empty())
			{
				if (Image const img{rootPath / convMat.OpacityMap, true}; !img.Empty())
				{
					mat->OpacityMap = std::make_shared<Texture>(img);
				}
			}

			if (!convMat.DiffuseMap.empty())
			{
				if (Image img{rootPath / convMat.DiffuseMap, true}; !img.Empty())
				{
					// If the diffuse map has an alpha channel, and we couldn't parse an opacity map
					// We assume that the transparency comes from the diffuse alpha, so we steal it
					// And create an opacity map from that.
					if (img.Channels() == 4 && !mat->OpacityMap)
					{
						auto alphaChan = img.ExtractChannel(3);
						mat->OpacityMap = std::make_shared<Texture>(alphaChan);
					}

					mat->DiffuseMap = std::make_shared<Texture>(img);
				}
			}

			if (!convMat.SpecularMap.empty())
			{
				if (Image const img{rootPath / convMat.SpecularMap, true}; !img.Empty())
				{
					mat->SpecularMap = std::make_shared<Texture>(img);
				}
			}

			return mat;
		}


		auto Parse(std::filesystem::path const& path) -> std::vector<Mesh>
		{
			auto const& [convMeshes, convMats] = convert::Import(path);

			std::vector<std::shared_ptr<Material>> mats;
			mats.reserve(convMats.size());

			for (auto const& convMat : convMats)
			{
				mats.push_back(ConvertMaterial(convMat, path.parent_path()));
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
