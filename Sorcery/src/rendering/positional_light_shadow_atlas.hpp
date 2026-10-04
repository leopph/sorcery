#pragma once

#include "shadow_atlas.hpp"
#include "wand/wand.hpp"

#include <array>


namespace sorcery::rendering {
class PositionalLightShadowAtlas final : public ShadowAtlas {
  std::array<Cell, 4> cells_;

public:
  PositionalLightShadowAtlas(wand::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
  [[nodiscard]] auto GetCell(int idx) -> Cell& override;
};
}
