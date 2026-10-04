#include "directional_light_shadow_map_array.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
DirectionalLightShadowMapArray::DirectionalLightShadowMapArray(wand::GraphicsDevice* const device,
                                                               DXGI_FORMAT const depth_format, UINT const size) :
  tex_{
    device->CreateTexture(wand::TextureDesc{
      .dimension = wand::TextureDimension::k2D,
      .width = size,
      .height = size,
      .depth_or_array_size = MAX_CASCADE_COUNT,
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
  size_{size} {}


auto DirectionalLightShadowMapArray::GetTex() const noexcept -> wand::SharedDeviceChildHandle<wand::Texture> const& {
  return tex_;
}


auto DirectionalLightShadowMapArray::GetSize() const noexcept -> UINT {
  return size_;
}
}
