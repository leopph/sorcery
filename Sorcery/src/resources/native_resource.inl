#pragma once

namespace sorcery {
template<std::derived_from<NativeResource> NativeResourceType>
auto CreateDeserializeInit(YAML::Node const& node) -> std::unique_ptr<NativeResourceType> {
  auto const res{Create<NativeResourceType>()};
  res->Deserialize(node);
  res->OnInit();
  return res;
}
}
