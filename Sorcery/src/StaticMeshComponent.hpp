#pragma once

#include "Component.hpp"
#include "Resources/Material.hpp"
#include "Resources/Mesh.hpp"

#include <span>
#include <vector>


namespace sorcery {
class StaticMeshComponent : public Component {
  RTTR_ENABLE(Component)
  std::vector<ResourceHandle<Material>> mMaterials;
  ResourceHandle<Mesh> mMesh;

  auto AdjustMaterialListForMesh() -> void;

public:
  LEOPPHAPI StaticMeshComponent();
  ~StaticMeshComponent() override;

  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::span<ResourceHandle<Material> const>;
  LEOPPHAPI auto SetMaterials(std::vector<ResourceHandle<Material>> materials) -> void;
  LEOPPHAPI auto ReplaceMaterial(int idx, ResourceHandle<Material> const& mtl) -> void;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> ResourceHandle<Mesh> const&;
  LEOPPHAPI auto SetMesh(ResourceHandle<Mesh> const& mesh) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
  LEOPPHAPI static Type const SerializationType;

  [[nodiscard]] LEOPPHAPI auto CalculateBounds() const noexcept -> AABB;
};
}
