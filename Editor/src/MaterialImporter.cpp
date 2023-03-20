#include "MaterialImporter.hpp"

#include "Material.hpp"

#include <fstream>
#include <vector>

#include "Serialization.hpp"

namespace leopph::editor {
auto MaterialImporter::GetSupportedExtensions() const -> std::string {
	return "mtl";
}


auto MaterialImporter::Import(InputImportInfo const& importInfo, [[maybe_unused]] std::filesystem::path const& cacheDir) -> Object* {
	std::ifstream in{ importInfo.src, std::ios::binary };
	std::vector<std::uint8_t> fileData{ std::istreambuf_iterator{ in }, {} };
	std::span bytes{ fileData };

	auto const albedoVector{ BinarySerializer<Vector3>::Deserialize(bytes.first<sizeof(Vector3)>(), std::endian::little) };
	bytes = bytes.subspan(sizeof(Vector3));

	auto const metallic{ BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little) };
	bytes = bytes.subspan(sizeof(f32));

	auto const roughness{ BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little) };
	bytes = bytes.subspan(sizeof(f32));

	auto const ao{ BinarySerializer<f32>::Deserialize(bytes.first<sizeof(f32)>(), std::endian::little) };
	bytes = bytes.subspan(sizeof(f32));

	auto const sampleAlbedo{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleMetallic{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleRoughness{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const sampleAo{ static_cast<bool>(BinarySerializer<u8>::Deserialize(bytes.first<sizeof(u8)>())) };
	bytes = bytes.subspan(sizeof(u8));

	auto const parseNextMap{
		[&bytes]() -> Texture2D* {
			auto const guidStrLength{ BinarySerializer<u64>::Deserialize(bytes.first<sizeof(u64)>(), std::endian::little) };
			bytes = bytes.subspan(sizeof(u64));

			if (bytes.size() >= guidStrLength) {
				std::string guidStr;
				std::ranges::copy_n(std::begin(bytes), guidStrLength, std::back_inserter(guidStr));
				auto const ret{ dynamic_cast<Texture2D*>(Object::FindObjectByGuid(Guid::Parse(guidStr))) };
				bytes = bytes.subspan(guidStrLength);
				return ret;
			}

			return nullptr;
		}
	};

	auto const albedoMap{ sampleAlbedo ? parseNextMap() : nullptr };
	auto const metallicMap{ sampleMetallic ? parseNextMap() : nullptr };
	auto const roughnessMap{ sampleRoughness ? parseNextMap() : nullptr };
	auto const aoMap{ sampleAo ? parseNextMap() : nullptr };

	return new Material{ albedoVector, metallic, roughness, ao, albedoMap, metallicMap, roughnessMap, aoMap };
}


auto MaterialImporter::GetPrecedence() const noexcept -> int {
	return 1;
}
}
