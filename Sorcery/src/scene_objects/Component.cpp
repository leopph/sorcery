#include "Component.hpp"

#include <iostream>
#include <cassert>

#include "Entity.hpp"
#include "TransformComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{"Component"}
    .property("entity", &sorcery::Component::entity_);
}


namespace sorcery {
auto Component::OnDrawProperties([[maybe_unused]] bool& changed) -> void {
  // We explicitly do not call SceneObject::OnDrawProperties here to avoid displaying the name and type
}


auto Component::OnAfterAttachedToEntity(Entity& entity) -> void {
  entity_.Reset(std::addressof(entity));
}


auto Component::OnBeforeDetachedFromEntity(Entity& entity) -> void {
  entity_.Reset();
}


auto Component::GetEntity() const -> ObserverPtr<Entity> {
  return entity_;
}
}
