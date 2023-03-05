#include "TextureImporter.hpp"

namespace leopph::editor {
auto TextureImporter::GetSupportedExtensions() const -> std::string {
	return "png,jpg,jpeg,tga,bmp,psd,gif,hdr,pic";
}

auto TextureImporter::Import(std::filesystem::path const& src) const -> Image {
	return Image{ src };
}
}
