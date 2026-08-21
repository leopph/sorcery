#pragma once

#include "Resource.hpp"
#include "../resource_reference.hpp"
#include "../Serialization.hpp"


namespace sorcery {
class NativeResource : public Resource {
  RTTR_ENABLE(Resource)

public:
  [[nodiscard]] LEOPPHAPI virtual
  auto Serialize() const noexcept -> YAML::Node = 0;

  LEOPPHAPI virtual
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void = 0;
};
}
