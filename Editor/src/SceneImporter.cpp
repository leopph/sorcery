#include "SceneImporter.hpp"

#include "Scene.hpp"
#include "YamlInclude.hpp"

namespace leopph {
auto editor::SceneImporter::GetSupportedExtensions() const -> std::string {
	return "scene";
}

auto editor::SceneImporter::Import(InputImportInfo const& importInfo, std::filesystem::path const& cacheDir) -> Object* {
	auto const scene{ new Scene{} };
	//TODO
	return scene;
}

auto editor::SceneImporter::GetPrecedence() const noexcept -> int {
	return 2;
}
}
