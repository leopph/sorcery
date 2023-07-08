#pragma once

#include "ShadowAtlas.hpp"
#include "Visibility.hpp"
#include "LightComponents.hpp"

#include <array>


namespace sorcery {
class PunctualShadowAtlas final : public ShadowAtlas {
  std::array<Cell, 4> mCells;

public:
  PunctualShadowAtlas(ID3D11Device* device, int size);

  auto Update(std::span<LightComponent const* const> allLights, Visibility const& visibility, Camera const& cam, Matrix4 const& camViewProjMtx, float shadowDistance) -> void;

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
};
}
