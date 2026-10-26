#include "sorcery/scene/Component.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Component>{"Component"};
}


namespace sorcery {
auto Component::GetEntity() const -> ObserverPtr<Entity> {
  return entity_;
}
}
