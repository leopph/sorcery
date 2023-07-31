#pragma once

#include "Component.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"

#include <vector>


namespace sorcery {
class StaticMeshComponent : public Component {
  RTTR_ENABLE(Component)
  std::vector<ObserverPtr<Material>> mMaterials;
  ObserverPtr<Mesh> mMesh;

  auto AdjustMaterialListForMesh() -> void;

public:
  LEOPPHAPI StaticMeshComponent();
  ~StaticMeshComponent() override;

  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<ObserverPtr<Material>> const&;
  LEOPPHAPI auto SetMaterials(std::vector<ObserverPtr<Material>> const& materials) -> void;
  LEOPPHAPI auto ReplaceMaterial(int idx, Material& mtl) -> void;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> Mesh&;
  LEOPPHAPI auto SetMesh(Mesh& mesh) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto CalculateBounds() const noexcept -> AABB;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
