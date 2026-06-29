#include "resource_reference.hpp"

#include "Serialization.hpp"


namespace sorcery {
auto SerializeGlobalResourceId(ResourceId const& id) -> YAML::Node {
  if (!id.IsValid()) {
    return SerializeNullResourceId();
  }

  YAML::Node node;
  node["scope"] = "Global";
  node["guid"] = id.GetGuid();
  node["fileIdx"] = id.GetIdxInFile();
  return node;
}


auto SerializeLocalResourceId(int const idx) -> YAML::Node {
  if (idx < 0) {
    return SerializeNullResourceId();
  }

  YAML::Node node;
  node["scope"] = "Local";
  node["fileIdx"] = idx;
  return node;
}


auto SerializeNullResourceId() -> YAML::Node {
  YAML::Node node;
  node["scope"] = "Null";
  return node;
}


auto DeserializeResourceId(YAML::Node const& node, YamlDeserializeContext const& ctx) -> std::optional<ResourceId> {
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

      return ResourceId{guid, file_idx};
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

      return ResourceId{ctx.current_guid, file_idx};
    }

    if (scope == "Null") {
      return ResourceId::Invalid();
    }

    return std::nullopt;
  } catch (...) {
    return std::nullopt;
  }
}
}
