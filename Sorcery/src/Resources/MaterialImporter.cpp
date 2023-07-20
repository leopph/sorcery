#include "MaterialImporter.hpp"
#include "Material.hpp"
#include "Texture2D.hpp"
#undef FindResource
#include "../YamlInclude.hpp"
#include "../Serialization.hpp"
#include "../ResourceManager.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MaterialImporter>("Material Importer")
    .constructor();
}


namespace sorcery {
auto MaterialImporter::GetSupportedFileExtensions(std::vector<std::string>& out) -> void {
  out.emplace_back(".mtl");
}


auto MaterialImporter::Import(std::filesystem::path const& src) -> std::shared_ptr<Resource> {
  auto const node{ YAML::LoadFile(src.string()) };

  auto const albedoVector{ node["albedo"].as<Vector3>() };
  auto const metallic{ node["metallic"].as<float>() };
  auto const roughness{ node["roughness"].as<float>() };
  auto const ao{ node["ao"].as<float>() };

  auto const albedoMapGuid{ Guid::Parse(node["albedoMap"].as<std::string>()) };
  auto albedoMap{
    albedoMapGuid.IsValid()
      ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(albedoMapGuid).lock())
      : nullptr
  };

  auto const metallicMapGuid{ Guid::Parse(node["metallicMap"].as<std::string>()) };
  auto metallicMap{
    metallicMapGuid.IsValid()
      ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(metallicMapGuid).lock())
      : nullptr
  };

  auto const roughnessMapGuid{ Guid::Parse(node["roughnessMap"].as<std::string>()) };
  auto roughnessMap{
    roughnessMapGuid.IsValid()
      ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(roughnessMapGuid).lock())
      : nullptr
  };

  auto const aoMapGuid{ Guid::Parse(node["aoMap"].as<std::string>()) };
  auto aoMap{
    aoMapGuid.IsValid()
      ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(aoMapGuid).lock())
      : nullptr
  };

  auto const normalMapGuid{ Guid::Parse(node["normalMap"].as<std::string>()) };
  auto normalMap{
    normalMapGuid.IsValid()
      ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(normalMapGuid).lock())
      : nullptr
  };

  return std::make_unique<Material>(albedoVector, metallic, roughness, ao, std::move(albedoMap), std::move(metallicMap), std::move(roughnessMap), std::move(aoMap), std::move(normalMap));
}


auto MaterialImporter::GetPrecedence() const noexcept -> int {
  return 1;
}
}
