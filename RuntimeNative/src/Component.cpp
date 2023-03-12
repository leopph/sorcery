#include "Component.hpp"

#include <iostream>

#include "Entity.hpp"
#include "TransformComponent.hpp"


namespace leopph {
auto Component::GetEntity() const -> Entity* {
	return mEntity;
}


auto Component::SetEntity(Entity* const entity) -> void {
	mEntity = entity;
}


auto Component::Serialize(YAML::Node& node) const -> void {
	node["entity"] = GetEntity()->GetGuid().ToString();
}


auto Component::Deserialize(YAML::Node const& node) -> void {
	if (!node["entity"].IsScalar()) {
		std::cerr << "Failed to deserialize owning Entity of Component " << GetGuid().ToString() << ". Invalid data." << std::endl;
	}
	else {
		auto const guidStr{ node["entity"].as<std::string>() };
		auto const entity{ dynamic_cast<Entity*>(Object::FindObjectByGuid(Guid::Parse(guidStr))) };
		if (!entity) {
			std::cerr << "Failed to deserialize owning Entity of Component " << GetGuid().ToString() << ". Guid " << guidStr << " does not belong to any Entity." << std::endl;
		}
		else {
			SetEntity(entity);
		}
	}
}


namespace managedbindings {
auto GetComponentEntity(MonoObject* const component) -> MonoObject* {
	return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetEntity()->GetManagedObject();
}

auto GetComponentEntityTransform(MonoObject* component) -> MonoObject* {
	return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetEntity()->GetTransform().GetManagedObject();
}
}
}
