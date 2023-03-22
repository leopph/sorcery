#include "Texture2DImporter.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture2D.hpp"

namespace leopph::editor {
auto Texture2DImporter::GetSupportedExtensions() const -> std::string {
	return "png,jpg,jpeg,tga,bmp,psd,gif,hdr,pic";
}

auto Texture2DImporter::Import(Importer::InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	int width;
	int height;
	int channelCount;

	if (auto const imgData{ stbi_load(importInfo.src.string().c_str(), &width, &height, &channelCount, 4) }) {
		return new Texture2D{ Image{ static_cast<u32>(width), static_cast<u32>(height), 4, std::unique_ptr<u8[]>{ imgData } } };
	}

	return nullptr;
}

auto Texture2DImporter::GetPrecedence() const noexcept -> int {
	return 0;
}
}