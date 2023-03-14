#include "MaterialImporter.hpp"

#include "Material.hpp"

#include <fstream>
#include <vector>

namespace leopph::editor {
auto MaterialImporter::GetSupportedExtensions() const -> std::string {
	return "mtl";
}

auto MaterialImporter::Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	auto const material{ new Material{} };

	std::ifstream in{ importInfo.src, std::ios::binary };
	std::vector<std::uint8_t> fileData{ std::istreambuf_iterator{ in }, {} };

	material->Deserialize(fileData);

	return material;
}

auto MaterialImporter::GetPrecedence() const noexcept -> int {
	return 1;
}
}
