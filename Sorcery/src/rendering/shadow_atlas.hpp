#pragma once

#include "wand/wand.hpp"


namespace sorcery::rendering {
class ShadowAtlas {
public:
  ShadowAtlas(wand::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  ShadowAtlas(ShadowAtlas const&) = delete;
  ShadowAtlas(ShadowAtlas&&) = delete;

  ~ShadowAtlas() = default;

  auto operator=(ShadowAtlas const&) -> void = delete;
  auto operator=(ShadowAtlas&&) -> void = delete;

  [[nodiscard]]
  auto GetTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const&;
  [[nodiscard]]
  auto GetSize() const noexcept -> UINT;

private:
  wand::SharedDeviceHandle<wand::Texture> tex_;
  UINT size_;
};
}
