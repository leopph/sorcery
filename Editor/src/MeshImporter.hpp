#pragma once

#include <string>
#include <memory>
#include <filesystem>

#include "Mesh.hpp"


namespace leopph::editor {
class MeshImporter {
	class Impl;
	Impl* mImpl;

public:
	MeshImporter();
	~MeshImporter();

	[[nodiscard]] auto GetSupportedExtensions() const -> std::string;
	[[nodiscard]] auto Import(std::filesystem::path const& path) const -> Mesh::Data;
};
}
