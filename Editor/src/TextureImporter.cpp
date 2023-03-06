#include "TextureImporter.hpp"

#include "Texture2D.hpp"

namespace leopph::editor {
auto TextureImporter::GetSupportedExtensions() const -> std::string {
	return "png,jpg,jpeg,tga,bmp,psd,gif,hdr,pic";
}

auto TextureImporter::Import(Importer::InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	return new Texture2D{ Image{ importInfo.src } };
}

auto TextureImporter::GetPrecedence() const noexcept -> int {
	return 0;
}
}
