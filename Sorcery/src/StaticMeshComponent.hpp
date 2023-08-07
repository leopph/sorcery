#pragma once

#include "Component.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"

#include <vector>


namespace sorcery {
class StaticMeshComponent : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND
  std::vector<ObserverPtr<Material>> mMaterials;
  ObserverPtr<Mesh> mMesh;

  auto ResizeMaterialListToSubmeshCount() -> void;

public:
  LEOPPHAPI StaticMeshComponent();
  ~StaticMeshComponent() override;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> ObserverPtr<Mesh>;
  LEOPPHAPI auto SetMesh(ObserverPtr<Mesh> mesh) noexcept -> void;

  // The returned vector is the same length as the Mesh's submesh count.
  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<ObserverPtr<Material>> const&;
  LEOPPHAPI auto SetMaterials(std::vector<ObserverPtr<Material>> const& materials) -> void;
  LEOPPHAPI auto SetMaterial(int idx, ObserverPtr<Material> mtl) -> void;

  [[nodiscard]] LEOPPHAPI auto CalculateBounds() const noexcept -> AABB;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
