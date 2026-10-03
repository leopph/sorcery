#include "MeshComponentBase.hpp"

#include <format>
#include <stdexcept>

#include "Entity.hpp"
#include "../app.hpp"
#include "../resource_manager.hpp"
#include "../rendering/scene_renderer.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshComponentBase>{"Mesh Component Base"}
    .property("mesh", &sorcery::MeshComponentBase::GetMesh, &sorcery::MeshComponentBase::SetMesh)
    .property("materials", &sorcery::MeshComponentBase::GetMaterials, &sorcery::MeshComponentBase::SetMaterials);
}


namespace sorcery {
auto MeshComponentBase::OnDrawGizmosSelected() -> void {
  Component::OnDrawGizmosSelected();

  auto const mesh = mesh_.Get().Get();

  if (!show_bounding_boxes_ | !mesh) {
    return;
  }

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

  if (auto const submesh_count = mesh->GetSubmeshes().size(); submesh_count > 1) {
    for (auto i{0uz}; i < submesh_count; i++) {
      draw_aabb_edges(mesh->GetSubmeshes()[i].GetBounds().Transform(local_to_world_mtx), Color{255, 165, 0, 255});
    }
  }

  draw_aabb_edges(mesh->GetBounds().Transform(local_to_world_mtx), Color::Red());
}


MeshComponentBase::MeshComponentBase() :
  mesh_{App::Instance().GetResourceManager().GetCubeMesh()} {
  ResizeMaterialListToSlotCount();
}


MeshComponentBase::~MeshComponentBase() = default;


auto MeshComponentBase::GetMesh() const noexcept -> ResourceRef<Mesh> {
  return mesh_;
}


auto MeshComponentBase::SetMesh(ResourceRef<Mesh> const mesh) noexcept -> void {
  mesh_ = mesh;
  ResizeMaterialListToSlotCount();
}


auto MeshComponentBase::GetMaterials() const noexcept -> std::vector<ResourceRef<Material>> const& {
  return materials_;
}


auto MeshComponentBase::SetMaterials(std::vector<ResourceRef<Material>> const& materials) -> void {
  materials_ = materials;
  ResizeMaterialListToSlotCount();
}


auto MeshComponentBase::SetMaterial(int const idx, ResourceRef<Material> const& mtl) -> void {
  if (idx >= std::ssize(materials_)) {
    throw std::runtime_error{
      std::format("Invalid index {} while attempting to replace material on mesh component.", idx)
    };
  }

  materials_[idx] = mtl;
}


auto MeshComponentBase::IsShowingBoundingBoxes() -> bool {
  return show_bounding_boxes_;
}


auto MeshComponentBase::SetShowBoundingBoxes(bool const show) -> void {
  show_bounding_boxes_ = show;
}


auto MeshComponentBase::ResizeMaterialListToSlotCount() -> void {
  auto const mesh = mesh_.Get().Get();

  if (!mesh) {
    materials_.clear();
    return;
  }

  if (auto const slot_count = mesh->GetMaterialSlots().size(), mtl_count = materials_.size();
    slot_count != mtl_count) {
    materials_.resize(slot_count);

    for (std::size_t i{mtl_count}; i < slot_count; i++) {
      materials_[i] = App::Instance().GetResourceManager().GetDefaultMaterial();
    }
  }
}


bool MeshComponentBase::show_bounding_boxes_{false};
}
