#pragma once

#include <memory>
#include <span>
#include <vector>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include "Core.hpp"
#include "asset_reference.hpp"


namespace sorcery {
class Entity;


struct EntitySerializationContext {
  AssetRefSerialization asset_ref_serialization{
    AssetRefSerialization::kGlobal
  };
};


[[nodiscard]] SORCERYAPI
auto SerializeEntitySet(
  std::span<Entity const* const> entities,
  EntitySerializationContext const& ctx = {}
) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeEntitySet(
  YAML::Node const& scene_objects_node,
  YamlDeserializeContext const& ctx
) -> std::vector<std::unique_ptr<Entity>>;
}
