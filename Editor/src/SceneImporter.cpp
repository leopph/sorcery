#include "SceneImporter.hpp"

#include "Scene.hpp"
#include <fstream>

namespace leopph {
auto editor::SceneImporter::GetSupportedExtensions() const -> std::string {
	return "scene";
}

auto editor::SceneImporter::Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	auto const scene{ new Scene{} };
	std::ifstream in{ importInfo.src, std::ios::in | std::ios::binary };
	std::vector<unsigned char> fileData{ std::istreambuf_iterator{ in }, {} };
	scene->Deserialize(fileData);
	return scene;
}

auto editor::SceneImporter::GetPrecedence() const noexcept -> int {
	return 2;
}
}
