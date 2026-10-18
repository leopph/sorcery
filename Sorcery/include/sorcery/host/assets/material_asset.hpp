#pragma once

#include <cstdint>
#include <optional>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include <sorcery/asset_id.hpp>
#include <sorcery/asset_reference.hpp>
#include <sorcery/Core.hpp>
#include <sorcery/material_blend_mode.hpp>
#include <sorcery/Math.hpp>


namespace sorcery {
struct MaterialAssetData {
  Vector3 base_color{1.0f, 1.0f, 1.0f};
  float metallic{0.0f};
  float roughness{0.5f};
  float ao{1.0f};
  Vector3 emissive{0.0f, 0.0f, 0.0f};
  float emissive_intensity{1.0f};
  float opacity{1.0f};
  std::uint32_t opacity_channel{0};

  MaterialBlendMode blend_mode{MaterialBlendMode::kOpaque};
  float alpha_threshold{0.5f};

  AssetId base_color_map{AssetId::Invalid()};
  AssetId metallic_map{AssetId::Invalid()};
  AssetId roughness_map{AssetId::Invalid()};
  AssetId ao_map{AssetId::Invalid()};
  AssetId normal_map{AssetId::Invalid()};
  AssetId opacity_map{AssetId::Invalid()};
  AssetId emissive_map{AssetId::Invalid()};
};


[[nodiscard]] SORCERYAPI
auto SerializeMaterialAssetData(
  MaterialAssetData const& data,
  AssetRefSerialization ref_serialization
) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeMaterialAssetData(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<MaterialAssetData>;
}
