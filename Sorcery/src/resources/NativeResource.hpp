#pragma once

#include "Resource.hpp"
#include "../resource_reference.hpp"
#include "../Serialization.hpp"

#include <memory>


namespace sorcery {
class NativeResource : public Resource {
  RTTR_ENABLE(Resource)

public:
  [[nodiscard]] LEOPPHAPI virtual
  auto Serialize() const noexcept -> YAML::Node = 0;

  LEOPPHAPI virtual
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void = 0;
};


template<std::derived_from<NativeResource> NativeResourceType>
[[nodiscard]]
auto CreateDeserialize(
  YAML::Node const& node,
  YamlDeserializeContext const& ctx
) -> std::unique_ptr<NativeResourceType>;
}


#include "native_resource.inl"
