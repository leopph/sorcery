#include "ImageSprite.hpp"

#include "../../data/DataManager.hpp"
#include "../../math/LeopphMath.hpp"
#include "../../util/Logger.hpp"

#include <vector>


namespace leopph
{
	ImageSprite::ImageSprite(std::filesystem::path const& src, unsigned const ppi) :
		RenderComponent{GetMeshGroup(src, ppi)}
	{}


	auto ImageSprite::GetMeshGroup(std::filesystem::path const& src, unsigned const ppi) -> std::shared_ptr<internal::MeshGroup const>
	{
		auto& dataManager = internal::DataManager::Instance();
		auto const meshId{GetMeshId(src, ppi)};

		// Try to find in cache.
		if (auto meshGroup = dataManager.FindMeshGroup(meshId))
		{
			return meshGroup;
		}

		// Build from zero

		Image img{src, true};

		if (img.Empty())
		{
			auto ret = std::make_shared<internal::MeshGroup>(meshId);
			dataManager.RegisterMeshGroup(ret);
			return ret;
		}

		std::shared_ptr<Texture> baseTexture;
		std::shared_ptr<Texture> opacityTexture;

		// If either side of the image is not power of two sized, we create a transparent bounding image and copy the image into its middle.
		if (!math::IsPowerOfTwo(img.Width()) || !math::IsPowerOfTwo(img.Height()))
		{
			auto const newDim{img.Width() > img.Height() ? math::NextPowerOfTwo(img.Width()) : math::NextPowerOfTwo(img.Height())};
			auto const startIndHoriz{static_cast<unsigned>(static_cast<float>(newDim - img.Width()) / 2.f)};
			auto const startIndVert{static_cast<unsigned>(static_cast<float>(newDim - img.Height()) / 2.f)};

			std::vector<unsigned char> baseColorBytes(newDim * newDim * 3, 0);
			std::vector<unsigned char> opacityBytes(newDim * newDim, 0);

			switch (img.Channels())
			{
				// We scale grayscale images up to RGB for the base color.
				// We fill the alpha channel width 255 where we store the image in the bounding image.
				case 1:
				{
					for (auto i = 0; i < img.Height(); i++)
					{
						for (auto j = 0; j < img.Width(); j++)
						{
							for (auto k = 0; k < 3; k++)
							{
								baseColorBytes[(startIndVert + i) * newDim * 3 + (startIndHoriz + j) * 3 + k] = img[i][j];
							}
							opacityBytes[(startIndVert + i) * newDim + (startIndHoriz + j)] = 255;
						}
					}
					break;
				}
				// We copy the color values from the RGB image to the base color image.
				// We fill the alpha channel width 255 where we store the image in the bounding image.
				case 3:
				{
					for (auto i = 0; i < img.Height(); i++)
					{
						for (auto j = 0; j < img.Width(); j++)
						{
							for (auto k = 0; k < 3; k++)
							{
								baseColorBytes[(startIndVert + i) * newDim * 3 + (startIndHoriz + j) * 3 + k] = img[i][j * 3 + k];
							}
							opacityBytes[(startIndVert + i) * newDim + (startIndHoriz + j)] = 255;
						}
					}
					break;
				}
				// We copy the RGB components from the RGBA image to the base color image.
				// We put the alpha values in the opacity image.
				case 4:
				{
					for (auto i = 0; i < img.Height(); i++)
					{
						for (auto j = 0; j < img.Width(); j++)
						{
							for (auto k = 0; k < 3; k++)
							{
								baseColorBytes[(startIndVert + i) * newDim * 3 + (startIndHoriz + j) * 3 + k] = img[i][j * 4 + k];
							}
							opacityBytes[(startIndVert + i) * newDim + (startIndHoriz + j)] = img[i][j * 4 + 3];
						}
					}
					break;
				}
				// We ignore weird images with 2 channels.
				default:
				{
					internal::Logger::Instance().Error("Failed to create sprite from image with " + std::to_string(img.Channels()) + " channels.");
					auto ret = std::make_shared<internal::MeshGroup>(meshId);
					dataManager.RegisterMeshGroup(ret);
					return ret;
				}
			}

			baseTexture = std::make_shared<Texture>(Image{static_cast<int>(newDim), static_cast<int>(newDim), 3, baseColorBytes});
			opacityTexture = std::make_shared<Texture>(Image{static_cast<int>(newDim), static_cast<int>(newDim), 1, opacityBytes});
		}
		else
		{
			if (img.Channels() == 4)
			{
				auto const opacityImg{img.ExtractChannel(3)};
				opacityTexture = std::make_shared<Texture>(opacityImg);
			}
			else
			{
				std::vector<unsigned char> alphaBytes(img.Width() * img.Height(), 255);
				opacityTexture = std::make_shared<Texture>(Image{img.Width(), img.Height(), 1, alphaBytes});
			}
			baseTexture = std::make_shared<Texture>(img);
		}

		// The width of the required rectangle.
		auto const rectW = static_cast<float>(baseTexture->Width()) / static_cast<float>(ppi);
		// The height of the required rectangle.
		auto const rectH = static_cast<float>(baseTexture->Height()) / static_cast<float>(ppi);

		std::vector vertices
		{
			internal::Vertex{Vector3{-rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{0, 1}}, // top left
			internal::Vertex{Vector3{-rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{0, 0}}, // bottom left
			internal::Vertex{Vector3{rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{1, 0}}, // bottom right
			internal::Vertex{Vector3{rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{1, 1}} // top right
		};

		std::vector<unsigned> indices
		{
			0, 1, 3, 1, 2, 3
		};

		auto material = std::make_shared<Material>(Color{255, 255, 255}, Color{0, 0, 0}, std::move(baseTexture), nullptr, opacityTexture, 0.f, 1.f, false);

		auto meshGroup = std::make_shared<internal::MeshGroup>(meshId, std::vector{internal::Mesh{std::move(vertices), std::move(indices), std::move(material)}});
		dataManager.RegisterMeshGroup(meshGroup);
		return meshGroup;
	}


	auto ImageSprite::GetMeshId(std::filesystem::path const& src, unsigned const ppi) -> std::string
	{
		return src.generic_string() + "+ppi=" + std::to_string(ppi);
	}
}
