#pragma once

#include <vector>

#include <sorcery/asset_ref.hpp>
#include <sorcery/assets/Material.hpp>
#include <sorcery/assets/Mesh.hpp>
#include <sorcery/scene/Component.hpp>


namespace sorcery {
class MeshComponentBase : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  LEOPPHAPI MeshComponentBase();
  LEOPPHAPI ~MeshComponentBase() override = 0;

  [[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> AssetRef<Mesh>;
  LEOPPHAPI virtual auto SetMesh(AssetRef<Mesh> mesh) noexcept -> void;

  // The returned vector is the same length as the material slot list of the mesh.
  [[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::vector<AssetRef<Material>> const&;
  LEOPPHAPI auto SetMaterials(std::vector<AssetRef<Material>> const& materials) -> void;
  LEOPPHAPI auto SetMaterial(int idx, AssetRef<Material> const& mtl) -> void;

  [[nodiscard]] SORCERYAPI static
  auto IsShowingBoundingBoxes() -> bool;

  SORCERYAPI static
  auto SetShowBoundingBoxes(bool show) -> void;

private:
  auto ResizeMaterialListToSlotCount() -> void;

  std::vector<AssetRef<Material>> materials_;
  AssetRef<Mesh> mesh_;

  static bool show_bounding_boxes_; // TODO this should be stripped when not compiling for Mage
};
}
