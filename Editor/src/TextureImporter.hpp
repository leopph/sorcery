#pragma once

#include "Image.hpp"

namespace leopph::editor {
class TextureImporter {
public:
	[[nodiscard]] auto GetSupportedExtensions() const -> std::string;
	[[nodiscard]] auto Import(std::filesystem::path const& src) const -> Image;
};
}
