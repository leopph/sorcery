#include "Component.hpp"

#include "Entity.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{"Component"};
}


namespace sorcery {
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
