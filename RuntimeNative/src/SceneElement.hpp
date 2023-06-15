#pragma once

#include "Object.hpp"
#include "YamlInclude.hpp"


namespace leopph {
class SceneElement : public Object {
  RTTR_ENABLE(Object)

public:
  LEOPPHAPI virtual auto Serialize(YAML::Node& node) const -> void = 0;
  LEOPPHAPI virtual auto Deserialize(YAML::Node const& node) -> void = 0;
};
}
