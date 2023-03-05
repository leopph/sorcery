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


	[[nodiscard]] virtual auto Import(std::filesystem::path const& src) -> Object* = 0;
	[[nodiscard]] virtual auto GetPrecedence() const noexcept -> int = 0;
};
}
