#include "sorcery/scene/StaticMeshComponent.hpp"

#include "sorcery/host/app.hpp"
#include "sorcery/host/rendering/render_scene.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto StaticMeshComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<StaticMeshComponent>(*this);
}


auto StaticMeshComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetRenderScene().Register(*this);
}


auto StaticMeshComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetRenderScene().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
}
}
