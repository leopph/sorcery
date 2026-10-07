#include "shadow_map_array.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
ShadowMapArray::ShadowMapArray(wand::GraphicsDevice* const device, DXGI_FORMAT const depth_format, UINT const tex_size,
                               UINT16 const array_size) :
  tex_{
    device->CreateTexture(wand::TextureDesc{
      .dimension = wand::TextureDimension::k2D,
      .width = tex_size,
      .height = tex_size,
      .depth_or_array_size = array_size,
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
  tex_size_{tex_size},
  array_size_{array_size} {}


auto ShadowMapArray::GetTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const& {
  return tex_;
}


auto ShadowMapArray::GetTexSize() const noexcept -> UINT {
  return tex_size_;
}


auto ShadowMapArray::GetArraySize() const noexcept -> UINT16 {
  return array_size_;
}
}
