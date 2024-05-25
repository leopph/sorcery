#pragma once

#include "Resource.hpp"
#include "../Serialization.hpp"

#include <memory>


namespace sorcery {
class NativeResource : public Resource {
  RTTR_ENABLE(Resource)

public:
  [[nodiscard]] LEOPPHAPI virtual auto Serialize() const noexcept -> YAML::Node = 0;
  LEOPPHAPI virtual auto Deserialize(YAML::Node const& yamlNode) noexcept -> void = 0;
};


template<std::derived_from<NativeResource> NativeResourceType>
[[nodiscard]] auto CreateDeserialize(YAML::Node const& node) -> std::unique_ptr<NativeResourceType>;
}


#include "native_resource.inl"
