#include "render_target.hpp"

#include <utility>


namespace sorcery::rendering {
auto RenderTarget::Desc::operator==(Desc const& other) const -> bool {
  return width == other.width && height == other.height && color_format == other.color_format && depth_stencil_format ==
         other.depth_stencil_format && sample_count == other.sample_count && enable_unordered_access == other.
         enable_unordered_access && (!color_format || color_clear_value == other.color_clear_value) && (
           !other.depth_stencil_format || (depth_clear_value == other.depth_clear_value && stencil_clear_value == other.
                                           stencil_clear_value));
}


auto RenderTarget::New(graphics::GraphicsDevice& device, Desc const& desc) -> std::unique_ptr<RenderTarget> {
  if (!desc.color_format && !desc.depth_stencil_format) {
    return nullptr;
  }

  graphics::SharedDeviceChildHandle<graphics::Texture> color_tex;
  graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex;

  if (desc.color_format) {
    CD3DX12_CLEAR_VALUE const clear_value{*desc.color_format, desc.color_clear_value.data()};

    color_tex = device.CreateTexture(graphics::TextureDesc{
      graphics::TextureDimension::k2D, desc.width, desc.height, 1, 1, *desc.color_format, desc.sample_count, false,
      true, true, desc.enable_unordered_access
    }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_RENDER_TARGET, &clear_value);

    color_tex->SetDebugName(desc.debug_name + L" - Color Texture");
  }

  if (desc.depth_stencil_format) {
    CD3DX12_CLEAR_VALUE const clear_value{*desc.depth_stencil_format, desc.depth_clear_value, desc.stencil_clear_value};

    depth_stencil_tex = device.CreateTexture(graphics::TextureDesc{
      graphics::TextureDimension::k2D, desc.width, desc.height, 1, 1, *desc.depth_stencil_format, desc.sample_count,
      true, false, true, desc.enable_unordered_access
    }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE, &clear_value);

    depth_stencil_tex->SetDebugName(desc.debug_name + L" - Depth-Stencil Texture");
  }

  return std::unique_ptr<RenderTarget>{new RenderTarget{desc, std::move(color_tex), std::move(depth_stencil_tex)}};
}


auto RenderTarget::GetDesc() const noexcept -> Desc const& {
  return desc_;
}


auto RenderTarget::GetColorTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return color_tex_;
}


auto RenderTarget::GetDepthStencilTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return depth_stencil_tex_;
}


RenderTarget::RenderTarget(Desc desc, graphics::SharedDeviceChildHandle<graphics::Texture> color_tex,
                           graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex) :
  desc_{std::move(desc)},
  color_tex_{std::move(color_tex)},
  depth_stencil_tex_{std::move(depth_stencil_tex)} {}
}
