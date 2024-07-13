#include "SkinnedMeshComponent.hpp"

#include <cmath>
#include <imgui.h>

#include "../app.hpp"
#include "../Timing.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SkinnedMeshComponent>{"Skinned Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto SkinnedMeshComponent::OnDrawProperties(bool& changed) -> void {
  MeshComponentBase::OnDrawProperties(changed);

  ImGui::TableNextColumn();
  ImGui::Text("Animation");

  ImGui::TableNextColumn();
  std::vector<char const*> items;
  items.emplace_back("None");

  if (auto const mesh{GetMesh()}) {
    for (auto const& [name, duration, ticks_per_second, node_anims] : mesh->GetAnimations()) {
      items.emplace_back(name.c_str());
    }
  }

  if (auto combo_idx{static_cast<int>(cur_animation_idx_ ? *cur_animation_idx_ + 1 : 0)};
    ImGui::Combo("##animCombo", &combo_idx, items.data(), static_cast<int>(items.size()))) {
    cur_animation_idx_ = combo_idx == 0 ? std::nullopt : std::make_optional(combo_idx - 1);
  }
}


auto SkinnedMeshComponent::OnDrawGizmosSelected() -> void {
  MeshComponentBase::OnDrawGizmosSelected();
}


auto SkinnedMeshComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<SkinnedMeshComponent>(*this);
}


auto SkinnedMeshComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetSceneRenderer().Register(*this);
}


auto SkinnedMeshComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetSceneRenderer().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
}


auto SkinnedMeshComponent::SetMesh(Mesh* const mesh) noexcept -> void {
  MeshComponentBase::SetMesh(mesh);

  if (mesh) {
    for (UINT i{0}; i < rendering::RenderManager::GetMaxFramesInFlight(); i++) {
      skinned_vertex_buffers_[i] = App::Instance().GetGraphicsDevice().CreateBuffer(
        graphics::BufferDesc{mesh->GetVertexCount() * sizeof(Vector4), sizeof(Vector4), false, true, true},
        D3D12_HEAP_TYPE_DEFAULT);

      skinned_normal_buffers_[i] = App::Instance().GetGraphicsDevice().CreateBuffer(
        graphics::BufferDesc{mesh->GetVertexCount() * sizeof(Vector4), sizeof(Vector4), false, true, true},
        D3D12_HEAP_TYPE_DEFAULT);

      skinned_tangent_buffers_[i] = App::Instance().GetGraphicsDevice().CreateBuffer(
        graphics::BufferDesc{mesh->GetVertexCount() * sizeof(Vector4), sizeof(Vector4), false, true, true},
        D3D12_HEAP_TYPE_DEFAULT);

      if (auto const bones{mesh->GetBones()}; !bones.empty()) {
        bone_matrix_buffers_[i] = App::Instance().GetGraphicsDevice().CreateBuffer(
          graphics::BufferDesc{mesh->GetBones().size() * sizeof(Matrix4), sizeof(Matrix4), false, false, true},
          D3D12_HEAP_TYPE_DEFAULT);
      }
    }

    cur_animation_time_ticks_ = 0;
    cur_anim_delta_time_ = 0;
    cur_animation_idx_ = mesh->GetAnimations().empty() ? std::nullopt : std::make_optional(0);
  } else {
    cur_animation_idx_.reset();
  }
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


auto SkinnedMeshComponent::GetSkinnedVertexBuffers() const noexcept -> std::span<
  graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()> {
  return skinned_vertex_buffers_;
}


auto SkinnedMeshComponent::GetSkinnedNormalBuffers() const noexcept -> std::span<graphics::SharedDeviceChildHandle<
    graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()> {
  return skinned_normal_buffers_;
}


auto SkinnedMeshComponent::GetSkinnedTangentBuffers() const noexcept -> std::span<graphics::SharedDeviceChildHandle<
    graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()> {
  return skinned_tangent_buffers_;
}


auto SkinnedMeshComponent::GetBoneMatrixBuffers() const noexcept -> std::span<
  graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()> {
  return bone_matrix_buffers_;
}


auto SkinnedMeshComponent::GetCurrentAnimation() const -> std::optional<Animation> {
  return cur_animation_idx_ ? std::make_optional(GetMesh()->GetAnimations()[*cur_animation_idx_]) : std::nullopt;
}


auto SkinnedMeshComponent::GetCurrentAnimationTime() const noexcept -> float {
  return cur_animation_time_ticks_;
}
}
