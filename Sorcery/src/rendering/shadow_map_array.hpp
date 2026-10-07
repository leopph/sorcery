#pragma once

#include "wand/wand.hpp"


namespace sorcery::rendering {
class ShadowMapArray {
public:
  explicit ShadowMapArray(wand::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT tex_size, UINT16 array_size);
  ShadowMapArray(ShadowMapArray const& other) = delete;
  ShadowMapArray(ShadowMapArray&& other) noexcept = delete;

  ~ShadowMapArray() = default;

  auto operator=(ShadowMapArray const& other) -> ShadowMapArray& = delete;
  auto operator=(ShadowMapArray&& other) noexcept -> ShadowMapArray& = delete;

  [[nodiscard]] auto GetTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const&;
  [[nodiscard]] auto GetTexSize() const noexcept -> UINT;
  [[nodiscard]] auto GetArraySize() const noexcept -> UINT16;

private:
  wand::SharedDeviceHandle<wand::Texture> tex_;
  UINT tex_size_;
  UINT16 array_size_;
};
}
