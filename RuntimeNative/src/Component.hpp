#pragma once

#include "ManagedAccessObject.hpp"


namespace leopph {
class Entity;
class TransformComponent;

class Component : public ManagedAccessObject {
	Entity* mEntity{ nullptr };

public:
	[[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity*;
	LEOPPHAPI auto SetEntity(Entity* entity) -> void;

	LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
	LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;
};


namespace managedbindings {
auto GetComponentEntity(MonoObject* component) -> MonoObject*;
auto GetComponentEntityTransform(MonoObject* component) -> MonoObject*;
}
}
