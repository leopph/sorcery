#include "Component.hpp"

#include <iostream>

#include "Entity.hpp"
#include "TransformComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{ "Component" };
}


namespace sorcery {
auto Component::GetEntity() const -> Entity* {
  return mEntity;
}


auto Component::SetEntity(Entity* const entity) -> void {
  mEntity = entity;
}
}
