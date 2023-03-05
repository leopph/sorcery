#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "Mesh.hpp"
#include "Importer.hpp"


namespace leopph::editor {
class MeshImporter : public Importer {
	class Impl;
	Impl* mImpl;

public:
	MeshImporter();
	~MeshImporter() override;

	[[nodiscard]] auto GetSupportedExtensions() const -> std::string;
	[[nodiscard]] auto Import(std::filesystem::path const& src) -> Object* override;
	[[nodiscard]] auto GetPrecedence() const noexcept -> int override;
};
}
