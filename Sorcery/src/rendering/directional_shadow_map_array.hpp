#pragma once

#include "graphics.hpp"


namespace sorcery {
class DirectionalShadowMapArray {
  graphics::UniqueHandle<graphics::Texture> tex_;
  UINT size_;

public:
  explicit DirectionalShadowMapArray(graphics::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  [[nodiscard]] auto GetTex() const noexcept -> graphics::Texture*;
  [[nodiscard]] auto GetSize() const noexcept -> UINT;
};
}
