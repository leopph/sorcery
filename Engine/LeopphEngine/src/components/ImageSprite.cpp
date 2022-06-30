#include "ImageSprite.hpp"

#include "Logger.hpp"
#include "Math.hpp"

#include <algorithm>
#include <memory>
#include <vector>


namespace leopph
{
	ImageSprite::ImageSprite(std::filesystem::path const& src, int const ppi) :
		m_Path{src}
	{
		Image img{src, true};
		m_Extents = Vector2{img.Width() / 2.f / ppi, img.Height() / 2.f / ppi};
		Init(CreateMeshGroup(img, ppi));
	}


	auto ImageSprite::Clone() const -> ComponentPtr<>
	{
		return CreateComponent<ImageSprite>(*this);
	}


	auto ImageSprite::Path() const noexcept -> std::filesystem::path const&
	{
		return m_Path;
	}


	auto ImageSprite::Extents() const noexcept -> Vector2 const&
	{
		return m_Extents;
	}


	auto ImageSprite::CreateMeshGroup(Image& img, int const ppi) -> MeshGroup
	{
		if (img.Empty())
		{
			return {};
		}

		std::shared_ptr<GlTexture> baseTexture;
		std::shared_ptr<GlTexture> opacityTexture;

		// If either side of the image is not power of two sized, we create a transparent bounding image and copy the image into its middle.
		if (!math::IsPowerOfTwo(img.Width()) || !math::IsPowerOfTwo(img.Height()))
		{
			auto const newDim{img.Width() > img.Height() ? math::NextPowerOfTwo(img.Width()) : math::NextPowerOfTwo(img.Height())};
			auto const startIndHoriz{static_cast<unsigned>(static_cast<float>(newDim - img.Width()) / 2.f)};
			auto const startIndVert{static_cast<unsigned>(static_cast<float>(newDim - img.Height()) / 2.f)};

			auto baseColorBytes = std::make_unique<unsigned char[]>(newDim * newDim * 3);
			auto opacityBytes = std::make_unique<unsigned char[]>(newDim * newDim);

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
					return {};
				}
			}

			baseTexture = std::make_shared<GlTexture>(Image{static_cast<int>(newDim), static_cast<int>(newDim), 3, std::move(baseColorBytes)});
			opacityTexture = std::make_shared<GlTexture>(Image{static_cast<int>(newDim), static_cast<int>(newDim), 1, std::move(opacityBytes)});
		}
		else
		{
			if (img.Channels() == 4)
			{
				auto const opacityImg{img.ExtractChannel(3)};
				opacityTexture = std::make_shared<GlTexture>(opacityImg);
			}
			else
			{
				auto alphaBytes = std::make_unique_for_overwrite<unsigned char[]>(img.Width() * img.Height());
				std::ranges::fill(alphaBytes.get(), alphaBytes.get() + img.Width() * img.Height(), static_cast<unsigned char>(255));
				opacityTexture = std::make_shared<GlTexture>(Image{img.Width(), img.Height(), 1, std::move(alphaBytes)});
			}
			baseTexture = std::make_shared<GlTexture>(img);
		}

		// The width of the required rectangle.
		auto const rectW = static_cast<float>(baseTexture->Width()) / static_cast<float>(ppi);
		// The height of the required rectangle.
		auto const rectH = static_cast<float>(baseTexture->Height()) / static_cast<float>(ppi);

		std::vector vertices
		{
			Vertex{Vector3{-rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{0, 1}}, // top left
			Vertex{Vector3{-rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{0, 0}}, // bottom left
			Vertex{Vector3{rectW / 2, -rectH / 2, 0}, Vector3::Backward(), Vector2{1, 0}}, // bottom right
			Vertex{Vector3{rectW / 2, rectH / 2, 0}, Vector3::Backward(), Vector2{1, 1}} // top right
		};

		std::vector<unsigned> indices
		{
			0, 1, 3, 1, 2, 3
		};

		auto material = std::make_shared<Material>(Color{255, 255, 255}, Color{0, 0, 0}, std::move(baseTexture), nullptr, opacityTexture, 0.f, 1.f, false);

		return MeshGroup{std::vector{Mesh{std::move(vertices), std::move(indices), std::move(material)}}};
	}
}
