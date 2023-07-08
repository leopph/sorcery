#pragma once

#include "ShadowAtlas.hpp"
#include "Visibility.hpp"
#include "LightComponents.hpp"


namespace sorcery {
class DirectionalShadowAtlas final : public ShadowAtlas {
  Cell mCell;

public:
  explicit DirectionalShadowAtlas(ID3D11Device* device, int size);

  auto Update(std::span<LightComponent const* const> allLights, Visibility const& visibility, Camera const& cam, ShadowCascadeBoundaries const& shadowCascadeBoundaries, float aspectRatio, int cascadeCount, bool useStableCascadeProjection) -> void;

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
};
}
