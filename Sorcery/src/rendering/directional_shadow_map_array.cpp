#include "directional_shadow_map_array.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
DirectionalShadowMapArray::DirectionalShadowMapArray(graphics::GraphicsDevice* const device,
                                                     DXGI_FORMAT const depth_format, UINT const size) :
  tex_{
    device->CreateTexture(
      graphics::TextureDesc{
        graphics::TextureDimension::k2D, size, size, MAX_CASCADE_COUNT, 1, depth_format, 1, true, false, true, false
      }, graphics::CpuAccess::kNone,
      std::array{D3D12_CLEAR_VALUE{.Format = depth_format, .DepthStencil = {0.0f, 0}}}.data())
  },
  size_{size} {}


auto DirectionalShadowMapArray::GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return tex_;
}


auto DirectionalShadowMapArray::GetSize() const noexcept -> UINT {
  return size_;
}
}
