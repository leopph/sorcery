#include "MaterialImporter.hpp"

#include "Material.hpp"
#include "BinarySerializer.hpp"

#include <fstream>
#include <span>
#include <vector>

namespace leopph::editor {
auto MaterialImporter::GetSupportedExtensions() const -> std::string {
	return "mtl";
}

auto MaterialImporter::Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	auto const material{ new Material{} };

	std::ifstream in{ importInfo.src, std::ios::binary };
	std::vector<unsigned char> fileData{ std::istreambuf_iterator{ in }, {} };
	std::span const bytes{ fileData };

	material->SetAlbedoVector(BinarySerializer<Vector3>::Deserialize(bytes.first<sizeof(Vector3)>(), std::endian::native));
	material->SetMetallic(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3), sizeof(f32)>(), std::endian::native));
	material->SetRoughness(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + sizeof(f32), sizeof(f32)>(), std::endian::native));
	material->SetAo(BinarySerializer<f32>::Deserialize(bytes.subspan<sizeof(Vector3) + 2 * sizeof(f32), sizeof(f32)>(), std::endian::native));

	return material;
}

auto MaterialImporter::GetPrecedence() const noexcept -> int {
	return 1;
}
}
