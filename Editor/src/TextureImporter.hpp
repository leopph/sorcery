#pragma once

#include "Importer.hpp"

namespace leopph::editor {
class TextureImporter : public Importer {
public:
	[[nodiscard]] auto GetSupportedExtensions() const -> std::string;
	[[nodiscard]] auto Import(std::filesystem::path const& src) -> Object* override;
	[[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
