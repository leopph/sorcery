#pragma once

namespace sorcery {
template<std::derived_from<NativeResource> NativeResourceType>
auto CreateDeserialize(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::unique_ptr<NativeResourceType> {
  auto res{Create<NativeResourceType>()};
  res->Deserialize(node, ctx);
  return res;
}
}
