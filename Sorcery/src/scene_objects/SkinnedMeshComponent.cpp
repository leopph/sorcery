#include "SkinnedMeshComponent.hpp"

#include <cmath>

#include "../app.hpp"
#include "../Timing.hpp"
#include "../rendering/scene_renderer.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SkinnedMeshComponent>{"Skinned Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto SkinnedMeshComponent::OnDrawGizmosSelected() -> void {
  MeshComponentBase::OnDrawGizmosSelected();
}


auto SkinnedMeshComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<SkinnedMeshComponent>(*this);
}


auto SkinnedMeshComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetSceneRenderer().Register(*this);
}


auto SkinnedMeshComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetSceneRenderer().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
}


auto SkinnedMeshComponent::SetMesh(ResourceRef<Mesh> const mesh) noexcept -> void {
  MeshComponentBase::SetMesh(mesh);

  auto const mesh_inst = mesh.Get().Get();

  if (!mesh_inst) {
    cur_animation_idx_.reset();
    return;
  }

  cur_animation_time_ticks_ = 0;
  cur_anim_delta_time_ = 0;
  cur_animation_idx_ = mesh_inst->GetAnimations().empty() ? std::nullopt : std::make_optional(0);
}


auto SkinnedMeshComponent::Start() -> void {
  MeshComponentBase::Start();
  cur_animation_time_ticks_ = 0;
  cur_anim_delta_time_ = 0;
}


auto SkinnedMeshComponent::Update() -> void {
  if (auto const mesh{GetMesh()}; mesh && cur_animation_idx_ && *cur_animation_idx_ < mesh->GetAnimations().size()) {
    auto const& [name, duration, ticks_per_second, node_anims]{mesh->GetAnimations()[*cur_animation_idx_]};
    auto const actual_ticks_per_second{ticks_per_second == 0 ? 25.0f : ticks_per_second};
    cur_anim_delta_time_ += timing::GetFrameTime();
    cur_animation_time_ticks_ = std::fmod(cur_anim_delta_time_ * actual_ticks_per_second, duration);
  }
}


SkinnedMeshComponent::SkinnedMeshComponent() {
  SetUpdatable(true);
}


auto SkinnedMeshComponent::GetCurrentAnimationIndex() const -> std::optional<std::size_t> {
  return cur_animation_idx_;
}


auto SkinnedMeshComponent::SetCurrentAnimationIndex(std::optional<size_t> const idx) -> void {
  if (!idx || *idx < GetMesh()->GetAnimations().size()) {
    cur_animation_idx_ = idx;
  }
}


auto SkinnedMeshComponent::GetCurrentAnimation() const -> std::optional<Animation> {
  return cur_animation_idx_ ? std::make_optional(GetMesh()->GetAnimations()[*cur_animation_idx_]) : std::nullopt;
}


auto SkinnedMeshComponent::GetCurrentAnimationTime() const noexcept -> float {
  return cur_animation_time_ticks_;
}
}
