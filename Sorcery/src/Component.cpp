#include "Component.hpp"

#include <iostream>
#include <cassert>

#include "Entity.hpp"
#include "TransformComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{ "Component" };
}


namespace sorcery {
auto Component::GetEntity() const -> Entity& {
  assert(mEntity);
  return *mEntity;
}


auto Component::SetEntity(Entity& entity) -> void {
  mEntity = std::addressof(entity);
}
}
