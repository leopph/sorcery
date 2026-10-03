#pragma once

#include <vector>

#include "Component.hpp"
#include "../resource_ref.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Mesh.hpp"


namespace sorcery {
class MeshComponentBase : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  LEOPPHAPI MeshComponentBase();
  LEOPPHAPI ~MeshComponentBase() override = 0;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> ResourceRef<Mesh>;
  LEOPPHAPI virtual auto SetMesh(ResourceRef<Mesh> mesh) noexcept -> void;

  // The returned vector is the same length as the material slot list of the mesh.
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

  static bool show_bounding_boxes_; // TODO this should be stripped when not compiling for Mage
};
}
