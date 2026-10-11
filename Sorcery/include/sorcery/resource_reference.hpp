#pragma once

#include <cstdint>
#include <optional>

// yaml-cpp incorrectly uses dllexport specifiers so we silence their warnings
#pragma warning (push)
#pragma warning (disable: 4251 4275)
#include <yaml-cpp/yaml.h>
#pragma warning (pop)

#include "Core.hpp"
#include "Guid.hpp"
#include "resource_id.hpp"


namespace sorcery {
enum class ResourceRefSerialization : std::uint8_t {
  kGlobal,
  kLocal
};


struct YamlSerializeContext {
  ResourceRefSerialization resource_ref_serialization{ResourceRefSerialization::kGlobal};
};


struct YamlDeserializeContext {
  Guid current_guid{Guid::Invalid()};
};


[[nodiscard]] SORCERYAPI
auto SerializeGlobalResourceId(ResourceId const& id) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto SerializeLocalResourceId(int idx) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto SerializeNullResourceId() -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeResourceId(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<ResourceId>;
}
