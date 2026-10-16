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
#include "asset_id.hpp"


namespace sorcery {
enum class AssetRefSerialization : std::uint8_t {
  kGlobal,
  kLocal
};


struct YamlSerializeContext {
  AssetRefSerialization asset_ref_serialization{AssetRefSerialization::kGlobal};
};


struct YamlDeserializeContext {
  Guid current_guid{Guid::Invalid()};
};


[[nodiscard]] SORCERYAPI
auto SerializeGlobalAssetId(AssetId const& id) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto SerializeLocalAssetId(int idx) -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto SerializeNullAssetId() -> YAML::Node;

[[nodiscard]] SORCERYAPI
auto DeserializeAssetId(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::optional<AssetId>;
}
