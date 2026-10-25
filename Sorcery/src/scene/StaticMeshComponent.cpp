#include "sorcery/scene/StaticMeshComponent.hpp"

#include "sorcery/world.hpp"
#include "sorcery/host/rendering/render_world.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto StaticMeshComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<StaticMeshComponent>(*this);
}


auto StaticMeshComponent::OnAfterEnteringWorld(World& world) -> void {
  Component::OnAfterEnteringWorld(world);
  world.GetRenderWorld().Register(*this);
}


auto StaticMeshComponent::OnBeforeExitingWorld(World& world) -> void {
  world.GetRenderWorld().Unregister(*this);
  Component::OnBeforeExitingWorld(world);
}
}
