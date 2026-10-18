#include "sorcery/host/assets/material_asset.hpp"

#include <cassert>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include <spdlog/spdlog.h>

#include "sorcery/Serialization.hpp"


namespace sorcery {
namespace {
constexpr auto kCurrentVersion = 2u;
constexpr std::string_view kVersionPropName{"version"};
constexpr std::string_view kBaseColorPropName{"baseColor"};
constexpr std::string_view kMetallicPropName{"metallic"};
constexpr std::string_view kRoughnessPropName{"roughness"};
constexpr std::string_view kAoPropName{"ao"};
constexpr std::string_view kBlendModePropName{"blendMode"};
constexpr std::string_view kAlphaThreshPropName{"alphaThresh"};
constexpr std::string_view kEmissivePropName{"emissive"};
constexpr std::string_view kEmissiveIntensityPropName{"emissiveIntensity"};
constexpr std::string_view kBaseColorMapPropName{"baseColorMap"};
constexpr std::string_view kMetallicMapPropName{"metallicMap"};
constexpr std::string_view kRoughnessMapPropName{"roughnessMap"};
constexpr std::string_view kAoMapPropName{"aoMap"};
constexpr std::string_view kNormalMapPropName{"normalMap"};
constexpr std::string_view kOpacityMapPropName{"opacityMask"};
constexpr std::string_view kEmissiveMapPropName{"emissiveMap"};
}


auto SerializeMaterialAssetData(
  MaterialAssetData const& data,
  AssetRefSerialization ref_serialization
) -> YAML::Node {
  YAML::Node node;

  node[kVersionPropName] = kCurrentVersion;

  node[kBaseColorPropName] = data.base_color;
  node[kMetallicPropName] = data.metallic;
  node[kRoughnessPropName] = data.roughness;
  node[kAoPropName] = data.ao;
  node[kBlendModePropName] = static_cast<std::uint32_t>(data.blend_mode);
  node[kAlphaThreshPropName] = data.alpha_threshold;
  node[kEmissivePropName] = data.emissive;
  node[kEmissiveIntensityPropName] = data.emissive_intensity;

  auto const serialize_asset_id = [ref_serialization](AssetId const& id) -> YAML::Node {
    switch (ref_serialization) {
      case AssetRefSerialization::kGlobal:
        return SerializeGlobalAssetId(id);
      case AssetRefSerialization::kLocal:
        return SerializeLocalAssetId(id.GetIdxInFile());
    }

    spdlog::error("Failed to serialize asset reference: unknown mode.");
    assert(false && "Unknown asset reference serialization mode!");
    return SerializeGlobalAssetId(AssetId::Invalid());
  };

  node[kBaseColorMapPropName] = serialize_asset_id(data.base_color_map);
  node[kMetallicMapPropName] = serialize_asset_id(data.metallic_map);
  node[kRoughnessMapPropName] = serialize_asset_id(data.roughness_map);
  node[kAoMapPropName] = serialize_asset_id(data.ao_map);
  node[kNormalMapPropName] = serialize_asset_id(data.normal_map);
  node[kOpacityMapPropName] = serialize_asset_id(data.opacity_map);
  node[kEmissiveMapPropName] = serialize_asset_id(data.emissive_map);

  return node;
}


auto DeserializeMaterialAssetData(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<MaterialAssetData> {
  auto const version_node = node[kVersionPropName];
  // Version 1 had no version field, so we treat the missing version field as version 1
  auto const version = version_node ? version_node.as<std::uint32_t>(1u) : 1u;

  auto const log_missing_prop = [version](std::string_view const prop_name) {
    spdlog::error("Failed to deserialize material asset: property \"{}\" missing but required in version {}.",
      prop_name, version);
  };

  auto const deserialize_prop = [&]<typename T> [[nodiscard]] (
    std::string_view const prop_name, T& prop_data,
    std::optional<std::uint32_t> const required_in_version
  ) -> bool {
    auto const prop_node = node[prop_name];

    if (required_in_version && version >= *required_in_version && !prop_node) {
      log_missing_prop(prop_name);
      return false;
    }

    if constexpr (std::same_as<T, AssetId>) {
      prop_data = DeserializeAssetId(prop_node, ctx).value_or(AssetId::Invalid());
    } else if constexpr (std::is_enum_v<T>) {
      prop_data = static_cast<T>(prop_node.as<std::uint32_t>(std::to_underlying(prop_data)));
    } else {
      prop_data = prop_node.as<T>(prop_data);
    }

    return true;
  };

  MaterialAssetData data;

  if (
    !deserialize_prop(kBaseColorPropName, data.base_color, 1) ||
    !deserialize_prop(kMetallicPropName, data.metallic, 1) ||
    !deserialize_prop(kRoughnessPropName, data.roughness, 1) ||
    !deserialize_prop(kAoPropName, data.ao, 1) ||
    !deserialize_prop(kEmissivePropName, data.emissive, 2) ||
    !deserialize_prop(kEmissiveIntensityPropName, data.emissive_intensity, 2) ||
    !deserialize_prop(kBlendModePropName, data.blend_mode, 1) ||
    !deserialize_prop(kAlphaThreshPropName, data.alpha_threshold, 1) ||
    !deserialize_prop(kBaseColorMapPropName, data.base_color_map, 1) ||
    !deserialize_prop(kMetallicMapPropName, data.metallic_map, 1) ||
    !deserialize_prop(kRoughnessMapPropName, data.roughness_map, 1) ||
    !deserialize_prop(kAoMapPropName, data.ao_map, 1) ||
    !deserialize_prop(kNormalMapPropName, data.normal_map, 1) ||
    !deserialize_prop(kOpacityMapPropName, data.opacity_map, 1) ||
    !deserialize_prop(kEmissiveMapPropName, data.emissive_map, 2)
  ) {
    return std::nullopt;
  }

  return data;
}
}
