#pragma once

#include "Importer.hpp"

namespace leopph::editor {
class TextureImporter : public Importer {
public:
	[[nodiscard]] auto GetSupportedExtensions() const -> std::string;
	[[nodiscard]] auto Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* override;
	[[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
