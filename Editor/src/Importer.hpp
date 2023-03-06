#pragma once

#include "Object.hpp"
#include <filesystem>

namespace leopph::editor {
class Importer {
public:
	Importer() = default;
	Importer(Importer const& other) = default;
	Importer(Importer&& other) noexcept = default;

	auto operator=(Importer const& other) -> Importer& = default;
	auto operator=(Importer&& other) noexcept -> Importer& = default;

	virtual ~Importer() = default;

	struct InputImportInfo {
		std::filesystem::path src;
		Guid guid;
	};

	[[nodiscard]] virtual auto GetSupportedExtensions() const -> std::string = 0;
	[[nodiscard]] virtual auto Import(InputImportInfo const& importInfo, std::filesystem::path const& cacheDir) -> Object* = 0;
	[[nodiscard]] virtual auto GetPrecedence() const noexcept -> int = 0;
};
}
