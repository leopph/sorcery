#pragma once

#include <optional>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include "Core.hpp"
#include "material_blend_mode.hpp"
#include "Math.hpp"
#include "resource_id.hpp"
#include "resource_reference.hpp"


namespace sorcery {
struct MaterialResourceData {
  Vector3 base_color{1.0f, 1.0f, 1.0f};
  float metallic{0.0f};
  float roughness{0.5f};
  float ao{1.0f};

  MaterialBlendMode blend_mode{MaterialBlendMode::kOpaque};
  float alpha_threshold{0.5f};

  ResourceId base_color_map{ResourceId::Invalid()};
  ResourceId metallic_map{ResourceId::Invalid()};
  ResourceId roughness_map{ResourceId::Invalid()};
  ResourceId ao_map{ResourceId::Invalid()};
  ResourceId normal_map{ResourceId::Invalid()};
  ResourceId opacity_map{ResourceId::Invalid()};
};


[[nodiscard]] SORCERYAPI
auto SerializeMaterialResourceData(
  MaterialResourceData const& data,
  ResourceRefSerialization ref_serialization
) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeMaterialResourceData(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<MaterialResourceData>;
}
