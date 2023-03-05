#include "TextureImporter.hpp"

#include "Texture2D.hpp"

namespace leopph::editor {
auto TextureImporter::GetSupportedExtensions() const -> std::string {
	return "png,jpg,jpeg,tga,bmp,psd,gif,hdr,pic";
}

auto TextureImporter::Import(std::filesystem::path const& src) -> Object* {
	return new Texture2D{ Image{ src } };
}

auto TextureImporter::GetPrecedence() const noexcept -> int {
	return 0;
}
}
