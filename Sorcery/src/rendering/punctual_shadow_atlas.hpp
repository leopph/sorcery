#pragma once

#include "graphics.hpp"
#include "shadow_atlas.hpp"

#include <array>


namespace sorcery::rendering {
class PunctualShadowAtlas final : public ShadowAtlas {
  std::array<Cell, 4> cells_;

public:
  PunctualShadowAtlas(graphics::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
  [[nodiscard]] auto GetCell(int idx) -> Cell& override;
};
}
