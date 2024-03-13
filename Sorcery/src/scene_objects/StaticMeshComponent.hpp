#pragma once

#include "Component.hpp"
#include "../Resources/Material.hpp"
#include "../Resources/Mesh.hpp"

#include <vector>


namespace sorcery {
class StaticMeshComponent : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND
  std::vector<Material*> mMaterials;
  Mesh* mMesh;

  auto ResizeMaterialListToSubmeshCount() -> void;

public:
  LEOPPHAPI StaticMeshComponent();

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> Mesh*;
  LEOPPHAPI auto SetMesh(Mesh* mesh) noexcept -> void;

  // The returned vector is the same length as the Mesh's submesh count.
  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<Material*> const&;
  LEOPPHAPI auto SetMaterials(std::vector<Material*> const& materials) -> void;
  LEOPPHAPI auto SetMaterial(int idx, Material* mtl) -> void;

  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
