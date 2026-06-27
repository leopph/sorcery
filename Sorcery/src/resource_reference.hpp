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
enum class SerializedResourceRefScope : std::uint8_t {
  kLocal,
  kGlobal
};


struct YamlDeserializeContext {
  Guid current_guid{Guid::Invalid()};
};


[[nodiscard]] SORCERYAPI
auto SerializeGlobalResourceId(ResourceId const& id) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto SerializeLocalResourceId(int idx) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeResourceId(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<ResourceId>;
}
