#include "CubemapImporter.hpp"

#include "Image.hpp"
#include "Cubemap.hpp"

#include <stb_image.h>

#include <array>
#include <algorithm>

namespace leopph {
auto editor::CubemapImporter::GetSupportedExtensions() const -> std::string {
	return "png,jpg,jpeg,tga,bmp,psd,gif,hdr,pic";
}

auto editor::CubemapImporter::Import(InputImportInfo const& importInfo, std::filesystem::path const& cacheDir) -> Object* {
	int width;
	int height;
	int channelCount;

	if (auto const data{ stbi_load(importInfo.src.string().c_str(), &width, &height, &channelCount, 4) }) {
		std::array<Image, 6> faceImgs;

		// 4:3
		if ((width - height) * 3 == height) {
			auto const faceSize{ width / 4 };

			if (!IsPowerOfTwo(faceSize)) {
				return nullptr;
			}
		}
		// 3:4
		else if ((height - width) * 3 == width) {
			auto const faceSize{ height / 4 };

			if (!IsPowerOfTwo(faceSize)) {
				return nullptr;
			}
		}
		// 6:1
		else if (width == 6 * height) {
			auto const faceSize{ width / 6 };

			if (!IsPowerOfTwo(faceSize)) {
				return nullptr;
			}

			for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
				auto bytes{ std::make_unique_for_overwrite<u8[]>(faceSize * faceSize * 4) };

				for (int i = 0; i < faceSize; i++) {
					std::ranges::copy_n(data + i * width, faceSize, bytes.get() + i * faceSize);
				}

				faceImgs[faceIdx] = Image{ static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::move(bytes) };
			}
		}
		// 1:6
		else if (height == 6 * width) {
			auto const faceSize{ height / 6 };

			if (!IsPowerOfTwo(faceSize)) {
				return nullptr;
			}

			for (int faceIdx = 0; faceIdx < 6; faceIdx++) {
				faceImgs[faceIdx] = Image{ static_cast<u32>(faceSize), static_cast<u32>(faceSize), 4, std::unique_ptr<u8[]>{ data + faceIdx * faceSize * faceSize * 4 } };
			}
		}
		else {
			stbi_image_free(data);
			return nullptr;
		}

		// Create the cubemap from the face imgs
		return new Cubemap{ faceImgs };
	}

	return nullptr;
}

auto editor::CubemapImporter::GetPrecedence() const noexcept -> int {
	return 0;
}
}
