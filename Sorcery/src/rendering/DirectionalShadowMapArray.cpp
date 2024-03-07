#include "DirectionalShadowMapArray.hpp"

#include "shaders/shader_interop.h"


namespace sorcery {
DirectionalShadowMapArray::DirectionalShadowMapArray(graphics::GraphicsDevice* const device, UINT const size) :
  tex_{
    device->CreateTexture(
      graphics::TextureDesc{
        graphics::TextureDimension::k2D, size, size, MAX_CASCADE_COUNT, 1, DXGI_FORMAT_D32_FLOAT, {1, 0},
        D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, true, false, true, false
      }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_COMMON, std::array{
        D3D12_CLEAR_VALUE{.Format = DXGI_FORMAT_D32_FLOAT, .DepthStencil = {0.0f, 0}}
      }.data())
  },
  size_{size} {}


auto DirectionalShadowMapArray::GetTex() const noexcept -> graphics::Texture* {
  return tex_.get();
}


auto DirectionalShadowMapArray::GetSize() const noexcept -> UINT {
  return size_;
}
}
