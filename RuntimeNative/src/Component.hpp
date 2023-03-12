#pragma once

#include "SceneElement.hpp"


namespace leopph {
class Entity;
class TransformComponent;

class Component : public SceneElement {
	Entity* mEntity{ nullptr };

public:
	[[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity*;
	LEOPPHAPI auto SetEntity(Entity* entity) -> void;

	LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
	LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;
};


namespace managedbindings {
auto GetComponentEntity(MonoObject* component) -> MonoObject*;
auto GetComponentEntityTransform(MonoObject* component) -> MonoObject*;
}
}
