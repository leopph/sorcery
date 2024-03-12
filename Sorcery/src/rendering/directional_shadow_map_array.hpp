#pragma once

#include "graphics.hpp"


namespace sorcery {
class DirectionalShadowMapArray {
  graphics::SharedDeviceChildHandle<graphics::Texture> tex_;
  UINT size_;

public:
  explicit DirectionalShadowMapArray(graphics::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  [[nodiscard]] auto GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;
  [[nodiscard]] auto GetSize() const noexcept -> UINT;
};
}
