#include "MaterialLoader.hpp"

#include "Material.hpp"
#include "Serialization.hpp"
#include "Util.hpp"

#include <array>
#include <cstdint>
#include <fstream>
#include <vector>


namespace sorcery::mage {
auto MaterialLoader::GetSupportedExtensions() const -> std::span<std::string const> {
  static std::array<std::string, 1> const extensions{
    "mtl"
  };

  return extensions;
}


auto MaterialLoader::Load(std::filesystem::path const& src, [[maybe_unused]] std::filesystem::path const& cache) -> std::unique_ptr<Object> {
  std::ifstream in{ src, std::ios::binary };
  std::vector<std::uint8_t> fileData{ std::istreambuf_iterator{ in }, {} };
  std::span<std::uint8_t const> bytes{ fileData };

  std::endian constexpr endianness{ std::endian::little };

  std::int8_t materialVersion;
  bytes = BinarySerializer<std::int8_t>::Deserialize(bytes, endianness, materialVersion);

  Vector3 albedoVector;
  bytes = BinarySerializer<Vector3>::Deserialize(bytes, endianness, albedoVector);

  float metallic;
  bytes = BinarySerializer<float>::Deserialize(bytes, endianness, metallic);

  float roughness;
  bytes = BinarySerializer<float>::Deserialize(bytes, endianness, roughness);

  float ao;
  bytes = BinarySerializer<float>::Deserialize(bytes, endianness, ao);

  bool sampleAlbedo;
  bytes = BinarySerializer<bool>::Deserialize(bytes, endianness, sampleAlbedo);

  bool sampleMetallic;
  bytes = BinarySerializer<bool>::Deserialize(bytes, endianness, sampleMetallic);

  bool sampleRoughness;
  bytes = BinarySerializer<bool>::Deserialize(bytes, endianness, sampleRoughness);

  bool sampleAo;
  bytes = BinarySerializer<bool>::Deserialize(bytes, endianness, sampleAo);

  bool sampleNormal;
  bytes = BinarySerializer<bool>::Deserialize(bytes, endianness, sampleNormal);

  auto const parseNextMap{
    [&bytes]() -> ObserverPtr<Texture2D> {
      std::string guidStr;
      bytes = BinarySerializer<std::string>::Deserialize(bytes, endianness, guidStr);
      return dynamic_cast<ObserverPtr<Texture2D>>(Object::FindObjectByGuid(Guid::Parse(guidStr)));
    }
  };

  auto const albedoMap{ sampleAlbedo ? parseNextMap() : nullptr };
  auto const metallicMap{ sampleMetallic ? parseNextMap() : nullptr };
  auto const roughnessMap{ sampleRoughness ? parseNextMap() : nullptr };
  auto const aoMap{ sampleAo ? parseNextMap() : nullptr };
  auto const normalMap{ sampleNormal ? parseNextMap() : nullptr };

  return std::make_unique<Material>(albedoVector, metallic, roughness, ao, albedoMap, metallicMap, roughnessMap, aoMap, normalMap);
}


auto MaterialLoader::GetPrecedence() const noexcept -> int {
  return 1;
}
}
