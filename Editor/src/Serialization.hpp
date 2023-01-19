#pragma once

#include <YamlInclude.hpp>
#include <Object.hpp>


namespace leopph::editor {
	[[nodiscard]] auto SerializeScene() -> YAML::Node;
	auto DeserializeScene(ObjectFactory const& factory, YAML::Node const& sceneNode) -> void;
}