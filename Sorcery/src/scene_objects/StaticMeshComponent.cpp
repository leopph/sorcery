#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "../app.hpp"
#include "../Gui.hpp"
#include "../rendering/scene_renderer.hpp"

#include <imgui.h>

#include <format>
#include <stdexcept>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticMeshComponent>{"Static Mesh Component"}
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("mesh", &sorcery::StaticMeshComponent::GetMesh, &sorcery::StaticMeshComponent::SetMesh)
    .property("materials", &sorcery::StaticMeshComponent::GetMaterials, &sorcery::StaticMeshComponent::SetMaterials);
}


namespace sorcery {
auto StaticMeshComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("%s", "Mesh");
  ImGui::TableNextColumn();
  static ObjectPicker<Mesh> meshPicker;
  if (auto mesh{GetMesh()}; meshPicker.Draw(mesh, true)) {
    SetMesh(mesh);
  }

  if (auto const mesh{GetMesh()}) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", "Materials");

    auto const mtlSlots{mesh->GetMaterialSlots()};
    auto const mtlCount{std::ssize(mtlSlots)};

    static std::vector<ObjectPicker<Material>> mtlPickers;
    if (std::ssize(mtlPickers) < mtlCount) {
      mtlPickers.resize(mtlCount);
    }

    auto const mtls{GetMaterials()};

    for (int i = 0; i < mtlCount; i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", mtlSlots[i].name.c_str());
      ImGui::TableNextColumn();
      if (auto mtl{mtls[i]}; mtlPickers[i].Draw(mtl, true)) {
        SetMaterial(i, mtl);
      }
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Show bounding boxes");
  ImGui::TableNextColumn();
  ImGui::Checkbox("##showAabbsCheckbox", &show_bounding_boxes_);
}


auto StaticMeshComponent::OnDrawGizmosSelected() -> void {
  Component::OnDrawGizmosSelected();

  if (show_bounding_boxes_) {
    if (mesh_) {
      auto const draw_aabb_edges{
        [](AABB const& aabb, Color const& line_color) {
          auto const& [min, max]{aabb};

          // Near face
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(min, Vector3{max[0], min[1], min[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(min, Vector3{min[0], max[1], min[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{max[0], max[1], min[2]},
            Vector3{max[0], min[1], min[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{max[0], max[1], min[2]},
            Vector3{min[0], max[1], min[2]}, line_color);

          // Far face
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{min[0], min[1], max[2]},
            Vector3{max[0], min[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{min[0], min[1], max[2]},
            Vector3{min[0], max[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(max, Vector3{max[0], min[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(max, Vector3{min[0], max[1], max[2]}, line_color);

          // Edges along Z
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(min, Vector3{min[0], min[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{max[0], min[1], min[2]},
            Vector3{max[0], min[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{min[0], max[1], min[2]},
            Vector3{min[0], max[1], max[2]}, line_color);
          App::Instance().GetSceneRenderer().DrawLineAtNextRender(Vector3{max[0], max[1], min[2]}, max, line_color);
        }
      };

      auto const& local_to_world_mtx{GetEntity()->GetTransform().GetLocalToWorldMatrix()};

      if (auto const drawable_submesh_count{std::max(mesh_->GetSubmeshCount(), static_cast<int>(materials_.size()))};
        drawable_submesh_count > 1) {
        for (auto i{0}; i < drawable_submesh_count; i++) {
          draw_aabb_edges(mesh_->GetSubMeshes()[i].bounds.Transform(local_to_world_mtx), Color{255, 165, 0, 255});
        }
      }

      draw_aabb_edges(mesh_->GetBounds().Transform(local_to_world_mtx), Color::Red());
    }
  }
}


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


StaticMeshComponent::StaticMeshComponent() :
  mesh_{App::Instance().GetResourceManager().GetCubeMesh()} {
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMesh() const noexcept -> Mesh* {
  return mesh_;
}


auto StaticMeshComponent::SetMesh(Mesh* const mesh) noexcept -> void {
  mesh_ = mesh;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::vector<Material*> const& {
  return materials_;
}


auto StaticMeshComponent::SetMaterials(std::vector<Material*> const& materials) -> void {
  materials_ = materials;
  ResizeMaterialListToSubmeshCount();
}


auto StaticMeshComponent::SetMaterial(int const idx, Material* const mtl) -> void {
  if (idx >= std::ssize(materials_)) {
    throw std::runtime_error{
      std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx)
    };
  }

  materials_[idx] = mtl;
}


auto StaticMeshComponent::ResizeMaterialListToSubmeshCount() -> void {
  if (!mesh_) {
    materials_.clear();
    return;
  }

  if (auto const subMeshCount{std::size(mesh_->GetSubMeshes())}, mtlCount{std::size(materials_)};
    subMeshCount != mtlCount) {
    materials_.resize(subMeshCount);

    for (std::size_t i{mtlCount}; i < subMeshCount; i++) {
      materials_[i] = App::Instance().GetResourceManager().GetDefaultMaterial().Get();
    }
  }
}


bool StaticMeshComponent::show_bounding_boxes_{false};
}
