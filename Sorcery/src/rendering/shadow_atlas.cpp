#include "shadow_atlas.hpp"

#include "../Math.hpp"
#include "shaders/shader_interop.h"


namespace sorcery::rendering {
ShadowAtlas::ShadowAtlas(wand::GraphicsDevice* const device, DXGI_FORMAT const depth_format, UINT const size) :
  tex_{
    device->CreateTexture(
      wand::TextureDesc{
        .dimension = wand::TextureDimension::k2D,
        .width = size,
        .height = size,
        .depth_or_array_size = 1,
        .mip_levels = 1,
        .format = depth_format,
        .sample_count = 1,
        .depth_stencil = true,
        .render_target = false,
        .shader_resource = true,
        .unordered_access = false
      }, wand::CpuAccess::kNone, std::array{
        D3D12_CLEAR_VALUE{
          .Format = depth_format,
          .DepthStencil = {
            .Depth = DEPTH_CLEAR_VALUE,
            .Stencil = 0
          }
        }
      }.data())
  },
  size_{size} {
  if (!IsPowerOfTwo(size_)) {
    throw std::runtime_error{"Shadow Atlas size must be power of 2."};
  }
}


auto ShadowAtlas::GetTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const& {
  return tex_;
}


auto ShadowAtlas::GetSize() const noexcept -> UINT {
  return size_;
}
}
