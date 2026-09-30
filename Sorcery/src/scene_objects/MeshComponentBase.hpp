#pragma once

#include <vector>

#include "Component.hpp"
#include "../resource_ref.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Mesh.hpp"


namespace sorcery {
class MeshComponentBase;


namespace detail {
[[nodiscard]]
auto GetPrevModelMtx(MeshComponentBase const& mesh_component) noexcept -> Matrix4 const&;
auto SetPrevModelMtx(MeshComponentBase& mesh_component, Matrix4 const& mtx) noexcept -> void;
}


class MeshComponentBase : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

  [[nodiscard]]
  friend auto detail::GetPrevModelMtx(MeshComponentBase const& mesh_component) noexcept -> Matrix4 const&;
  friend auto detail::SetPrevModelMtx(MeshComponentBase& mesh_component, Matrix4 const& mtx) noexcept -> void;

public:
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  LEOPPHAPI MeshComponentBase();
  LEOPPHAPI ~MeshComponentBase() override = 0;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> ResourceRef<Mesh>;
  LEOPPHAPI virtual auto SetMesh(ResourceRef<Mesh> mesh) noexcept -> void;

  // The returned vector is the same length as the Mesh's submesh count.
  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<ResourceRef<Material>> const&;
  LEOPPHAPI auto SetMaterials(std::vector<ResourceRef<Material>> const& materials) -> void;
  LEOPPHAPI auto SetMaterial(int idx, ResourceRef<Material> const& mtl) -> void;

  [[nodiscard]] SORCERYAPI static
  auto IsShowingBoundingBoxes() -> bool;

  SORCERYAPI static
  auto SetShowBoundingBoxes(bool show) -> void;

private:
  auto ResizeMaterialListToSlotCount() -> void;

  std::vector<ResourceRef<Material>> materials_;
  ResourceRef<Mesh> mesh_;
  Matrix4 prev_model_mtx_{Matrix4::Identity()};

  static bool show_bounding_boxes_; // TODO this should be stripped when not compiling for Mage
};
}
