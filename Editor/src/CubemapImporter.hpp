#pragma once

#include "Importer.hpp"

namespace leopph::editor {
class CubemapImporter : public Importer {
public:
	[[nodiscard]] auto GetSupportedExtensions() const -> std::string override;
	[[nodiscard]] auto Import(InputImportInfo const& importInfo, std::filesystem::path const& cacheDir) -> Object* override;
	[[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
