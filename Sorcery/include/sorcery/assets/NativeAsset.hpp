#pragma once

#include <sorcery/asset_reference.hpp>
#include <sorcery/Serialization.hpp>
#include <sorcery/assets/Asset.hpp>


namespace sorcery {
class NativeAsset : public Asset {
  RTTR_ENABLE(Asset)

public:
  [[nodiscard]] LEOPPHAPI virtual
  auto Serialize() const noexcept -> YAML::Node = 0;

  LEOPPHAPI virtual
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void = 0;
};
}
