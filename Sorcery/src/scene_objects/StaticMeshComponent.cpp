#include "StaticMeshComponent.hpp"

#include "../app.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto StaticMeshComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<StaticMeshComponent>(*this);
}


auto StaticMeshComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetSceneRenderer().Register(*this);
}


auto StaticMeshComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetSceneRenderer().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
}
}
