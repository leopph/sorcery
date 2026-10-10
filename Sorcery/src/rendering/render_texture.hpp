#pragma once

#include <DirectXTex.h>

#include "wand/wand.hpp"


namespace sorcery::rendering {
class RenderTexture {
public:
  auto Init(wand::GraphicsDevice& device, DirectX::TexMetadata const& metadata) -> void;

  [[nodiscard]]
  auto GetTexture() const -> wand::SharedDeviceHandle<wand::Texture> const&;

private:
  wand::SharedDeviceHandle<wand::Texture> tex_;
};
}
