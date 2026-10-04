#pragma once

#include "wand/wand.hpp"


namespace sorcery::rendering {
class DirectionalLightShadowMapArray {
public:
  explicit DirectionalLightShadowMapArray(wand::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);
  [[nodiscard]] auto GetTex() const noexcept -> wand::SharedDeviceChildHandle<wand::Texture> const&;
  [[nodiscard]] auto GetSize() const noexcept -> UINT;

private:
  wand::SharedDeviceChildHandle<wand::Texture> tex_;
  UINT size_;
};
}
