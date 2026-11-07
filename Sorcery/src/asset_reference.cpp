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
  node["fileIdx"] = *id.GetIdInFile();
  return node;
}


auto SerializeLocalAssetId(std::optional<std::uint64_t> const id_in_file) -> YAML::Node {
  if (!id_in_file) {
    return SerializeNullAssetId();
  }

  YAML::Node node;
  node["scope"] = "Local";
  node["fileIdx"] = *id_in_file;
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
      auto const file_id_node = node["fileIdx"];

      if (!guid_node || !file_id_node) {
        return std::nullopt;
      }

      auto const guid = guid_node.as<Guid>();

      if (!guid.IsValid()) {
        return std::nullopt;
      }

      auto const file_id = file_id_node.as<std::uint64_t>();
      return AssetId{guid, file_id};
    }

    if (scope == "Local") {
      if (!ctx.current_guid.IsValid()) {
        return std::nullopt;
      }

      auto const file_idx_node = node["fileIdx"];

      if (!file_idx_node) {
        return std::nullopt;
      }

      auto const file_id = file_idx_node.as<std::uint64_t>();
      return AssetId{ctx.current_guid, file_id};
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
