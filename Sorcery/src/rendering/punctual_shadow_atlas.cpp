#include "punctual_shadow_atlas.hpp"


namespace sorcery {
PunctualShadowAtlas::PunctualShadowAtlas(graphics::GraphicsDevice* const device, DXGI_FORMAT const depth_format,
                                         UINT const size):
  ShadowAtlas{device, depth_format, size, 2},
  cells_{Cell{1}, Cell{2}, Cell{4}, Cell{8}} {}


auto PunctualShadowAtlas::GetCell(int const idx) const -> Cell const& {
  ThrowIfIndexIsInvalid(idx);
  return cells_[idx];
}


auto PunctualShadowAtlas::GetCell(int const idx) -> Cell& {
  ThrowIfIndexIsInvalid(idx);
  return cells_[idx];
}
}
