#include "material_resource.hpp"

#include "Serialization.hpp"

#include <cassert>
#include <cstdint>


namespace sorcery {
auto SerializeMaterialResourceData(
  MaterialResourceData const& data,
  ResourceRefSerialization ref_serialization
) -> YAML::Node {
  YAML::Node node;

  node["baseColor"] = data.base_color;
  node["metallic"] = data.metallic;
  node["roughness"] = data.roughness;
  node["ao"] = data.ao;
  node["blendMode"] = static_cast<std::uint32_t>(data.blend_mode);
  node["alphaThresh"] = data.alpha_threshold;

  auto const serialize_res_id = [ref_serialization](ResourceId const& id) -> YAML::Node {
    switch (ref_serialization) {
      case ResourceRefSerialization::kGlobal:
        return SerializeGlobalResourceId(id);
      case ResourceRefSerialization::kLocal:
        return SerializeLocalResourceId(id.GetIdxInFile());
    }

    assert(false && "Unknown resource reference serialization mode!");
    return SerializeGlobalResourceId(ResourceId::Invalid());
  };

  node["baseColorMap"] = serialize_res_id(data.base_color_map);
  node["metallicMap"] = serialize_res_id(data.metallic_map);
  node["roughnessMap"] = serialize_res_id(data.roughness_map);
  node["aoMap"] = serialize_res_id(data.ao_map);
  node["normalMap"] = serialize_res_id(data.normal_map);
  node["opacityMask"] = serialize_res_id(data.opacity_map);

  return node;
}


auto DeserializeMaterialResourceData(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<MaterialResourceData> {
  auto const base_color_node = node["baseColor"];
  auto const metallic_node = node["metallic"];
  auto const roughness_node = node["roughness"];
  auto const ao_node = node["ao"];
  auto const blend_mode_node = node["blendMode"];
  auto const alpha_thresh_node = node["alphaThresh"];

  if (!base_color_node || !metallic_node || !roughness_node || !ao_node || !blend_mode_node || !alpha_thresh_node) {
    return std::nullopt;
  }

  MaterialResourceData data;
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

  data.base_color_map = DeserializeResourceId(base_color_map_node, ctx).value_or(ResourceId::Invalid());
  data.metallic_map = DeserializeResourceId(metallic_map_node, ctx).value_or(ResourceId::Invalid());
  data.roughness_map = DeserializeResourceId(roughness_map_node, ctx).value_or(ResourceId::Invalid());
  data.ao_map = DeserializeResourceId(ao_map_node, ctx).value_or(ResourceId::Invalid());
  data.normal_map = DeserializeResourceId(normal_map_node, ctx).value_or(ResourceId::Invalid());
  data.opacity_map = DeserializeResourceId(opacity_mask_node, ctx).value_or(ResourceId::Invalid());

  return data;
}
}
