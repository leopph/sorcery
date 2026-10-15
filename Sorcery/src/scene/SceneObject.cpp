#include "sorcery/scene/SceneObject.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SceneObject>{"Scene Object"}
    .property(
      "updatable",
      &sorcery::SceneObject::IsUpdatable,
      &sorcery::SceneObject::SetUpdatable
    )(
      rttr::metadata(
        sorcery::reflection::kHideInInspector,
        true
      )
    );
}


auto sorcery::SceneObject::IsUpdatable() const -> bool {
  return updatable_;
}


auto sorcery::SceneObject::SetUpdatable(bool const updatable) -> void {
  updatable_ = updatable;
}
