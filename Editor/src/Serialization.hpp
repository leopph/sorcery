#pragma once

#include <YamlInclude.hpp>
#include "ObjectFactoryManager.hpp"


namespace leopph::editor {
[[nodiscard]] auto SerializeScene() -> YAML::Node;
auto DeserializeScene(EditorObjectFactoryManager const& manager, YAML::Node const& sceneNode) -> void;
}
