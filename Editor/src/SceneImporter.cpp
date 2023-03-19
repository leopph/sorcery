#include "SceneImporter.hpp"

#include "Scene.hpp"

#include <fstream>
#include <vector>

namespace leopph::editor {
auto SceneImporter::GetSupportedExtensions() const -> std::string {
	return "scene";
}

auto SceneImporter::Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	auto const scene{ new Scene{} };

	std::ifstream in{ importInfo.src, std::ios::in | std::ios::binary };
	std::vector<std::uint8_t> fileData{ std::istreambuf_iterator{ in }, {} };

	if (!fileData.empty()) {
		scene->Deserialize(fileData);
	}

	return scene;
}

auto SceneImporter::GetPrecedence() const noexcept -> int {
	return 2;
}
}
