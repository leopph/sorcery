#include "sorcery/asset_reference.hpp"

#include "sorcery/Serialization.hpp"


namespace sorcery {
auto SerializeGlobalAssetId(AssetId const& id) -> YAML::Node {
  if (!id.IsValid()) {
    return SerializeNullAssetId();
  }

  YAML::Node node;
  node["scope"] = "Global";
  node["guid"] = id.GetGuid();
  node["fileIdx"] = id.GetIdxInFile();
  return node;
}


auto SerializeLocalAssetId(int const idx) -> YAML::Node {
  if (idx < 0) {
    return SerializeNullAssetId();
  }

  YAML::Node node;
  node["scope"] = "Local";
  node["fileIdx"] = idx;
  return node;
}


auto SerializeNullAssetId() -> YAML::Node {
  YAML::Node node;
  node["scope"] = "Null";
  return node;
}


auto DeserializeAssetId(YAML::Node const& node, YamlDeserializeContext const& ctx) -> std::optional<AssetId> {
  try {
    if (!node.IsMap()) {
      return std::nullopt;
    }

    auto const scope_node = node["scope"];

    if (!scope_node) {
      return std::nullopt;
    }

    auto const scope = scope_node.as<std::string>();

    if (scope == "Global") {
      auto const guid_node = node["guid"];
      auto const file_idx_node = node["fileIdx"];

      if (!guid_node || !file_idx_node) {
        return std::nullopt;
      }

      auto const guid = guid_node.as<Guid>();
      auto const file_idx = file_idx_node.as<int>();

      if (!guid.IsValid() || file_idx < 0) {
        return std::nullopt;
      }

      return AssetId{guid, file_idx};
    }

    if (scope == "Local") {
      auto const file_idx_node = node["fileIdx"];

      if (!file_idx_node) {
        return std::nullopt;
      }

      auto const file_idx = file_idx_node.as<int>();

      if (!ctx.current_guid.IsValid() || file_idx < 0) {
        return std::nullopt;
      }

      return AssetId{ctx.current_guid, file_idx};
    }

    if (scope == "Null") {
      return AssetId::Invalid();
    }

    return std::nullopt;
  } catch (...) {
    return std::nullopt;
  }
}
}
