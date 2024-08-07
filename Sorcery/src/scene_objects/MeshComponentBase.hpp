#pragma once

#include "Component.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Mesh.hpp"

#include <vector>


namespace sorcery {
class MeshComponentBase : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  LEOPPHAPI MeshComponentBase();
  LEOPPHAPI ~MeshComponentBase() override = 0;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> Mesh*;
  LEOPPHAPI virtual auto SetMesh(Mesh* mesh) noexcept -> void;

  // The returned vector is the same length as the Mesh's submesh count.
  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<Material*> const&;
  LEOPPHAPI auto SetMaterials(std::vector<Material*> const& materials) -> void;
  LEOPPHAPI auto SetMaterial(int idx, Material* mtl) -> void;

private:
  auto ResizeMaterialListToSubmeshCount() -> void;

  std::vector<Material*> materials_;
  Mesh* mesh_;

  static bool show_bounding_boxes_; // TODO this should be stripped when not compiling for Mage
};
}
