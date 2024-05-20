#include "Component.hpp"

#include <iostream>
#include <cassert>

#include "Entity.hpp"
#include "TransformComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{"Component"}
    .property("entity", &sorcery::Component::mEntity);
}


namespace sorcery {
Component::~Component() {
  mEntity->RemoveComponent(*this);
}


auto Component::GetEntity() const -> Entity& {
  assert(mEntity);
  return *mEntity;
}


auto Component::SetEntity(Entity& entity) -> void {
  mEntity = std::addressof(entity);
}


auto Component::OnDrawProperties([[maybe_unused]] bool& changed) -> void {
  // We explicitly do not call SceneObject::OnDrawProperties here to avoid displaying the name and type
}
}
