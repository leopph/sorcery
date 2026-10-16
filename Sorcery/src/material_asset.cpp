#include "sorcery/host/assets/material_asset.hpp"

#include <cassert>
#include <cstdint>

#include "sorcery/Serialization.hpp"


namespace sorcery {
auto SerializeMaterialAssetData(
  MaterialAssetData const& data,
  AssetRefSerialization ref_serialization
) -> YAML::Node {
  YAML::Node node;

  node["baseColor"] = data.base_color;
  node["metallic"] = data.metallic;
  node["roughness"] = data.roughness;
  node["ao"] = data.ao;
  node["blendMode"] = static_cast<std::uint32_t>(data.blend_mode);
  node["alphaThresh"] = data.alpha_threshold;

  auto const serialize_asset_id = [ref_serialization](AssetId const& id) -> YAML::Node {
    switch (ref_serialization) {
      case AssetRefSerialization::kGlobal:
        return SerializeGlobalAssetId(id);
      case AssetRefSerialization::kLocal:
        return SerializeLocalAssetId(id.GetIdxInFile());
    }

    assert(false && "Unknown asset reference serialization mode!");
    return SerializeGlobalAssetId(AssetId::Invalid());
  };

  node["baseColorMap"] = serialize_asset_id(data.base_color_map);
  node["metallicMap"] = serialize_asset_id(data.metallic_map);
  node["roughnessMap"] = serialize_asset_id(data.roughness_map);
  node["aoMap"] = serialize_asset_id(data.ao_map);
  node["normalMap"] = serialize_asset_id(data.normal_map);
  node["opacityMask"] = serialize_asset_id(data.opacity_map);

  return node;
}


auto DeserializeMaterialAssetData(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<MaterialAssetData> {
  auto const base_color_node = node["baseColor"];
  auto const metallic_node = node["metallic"];
  auto const roughness_node = node["roughness"];
  auto const ao_node = node["ao"];
  auto const blend_mode_node = node["blendMode"];
  auto const alpha_thresh_node = node["alphaThresh"];

  if (!base_color_node || !metallic_node || !roughness_node || !ao_node || !blend_mode_node || !alpha_thresh_node) {
    return std::nullopt;
  }

  MaterialAssetData data;
  data.base_color = base_color_node.as<Vector3>(data.base_color);
  data.metallic = metallic_node.as<float>(data.metallic);
  data.roughness = roughness_node.as<float>(data.roughness);
  data.ao = ao_node.as<float>(data.ao);
  data.blend_mode = static_cast<MaterialBlendMode>(blend_mode_node.as<std::uint32_t>());
  data.alpha_threshold = alpha_thresh_node.as<float>(data.alpha_threshold);

  auto const base_color_map_node = node["baseColorMap"];
  auto const metallic_map_node = node["metallicMap"];
  auto const roughness_map_node = node["roughnessMap"];
  auto const ao_map_node = node["aoMap"];
  auto const normal_map_node = node["normalMap"];
  auto const opacity_mask_node = node["opacityMask"];

  if (!base_color_map_node || !metallic_map_node || !roughness_map_node || !ao_map_node || !normal_map_node ||
      !opacity_mask_node) {
    return std::nullopt;
  }

  data.base_color_map = DeserializeAssetId(base_color_map_node, ctx).value_or(AssetId::Invalid());
  data.metallic_map = DeserializeAssetId(metallic_map_node, ctx).value_or(AssetId::Invalid());
  data.roughness_map = DeserializeAssetId(roughness_map_node, ctx).value_or(AssetId::Invalid());
  data.ao_map = DeserializeAssetId(ao_map_node, ctx).value_or(AssetId::Invalid());
  data.normal_map = DeserializeAssetId(normal_map_node, ctx).value_or(AssetId::Invalid());
  data.opacity_map = DeserializeAssetId(opacity_mask_node, ctx).value_or(AssetId::Invalid());

  return data;
}
}
