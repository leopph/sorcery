#pragma once

#include "ManagedAccessObject.hpp"
#include "YamlInclude.hpp"

namespace leopph {
class SceneElement : public ManagedAccessObject {
public:
	LEOPPHAPI virtual auto Serialize(YAML::Node& node) const -> void = 0;
	LEOPPHAPI virtual auto Deserialize(YAML::Node const& node) -> void = 0;
};
}
