#include "render_target.hpp"

#include <utility>


namespace sorcery::rendering {
auto RenderTarget::Desc::operator==(Desc const& other) const -> bool {
  return width == other.width && height == other.height && color_format == other.color_format && depth_stencil_format ==
         other.depth_stencil_format && sample_count == other.sample_count && enable_unordered_access == other.
         enable_unordered_access;
}


auto RenderTarget::New(graphics::GraphicsDevice& device, Desc const& desc) -> std::unique_ptr<RenderTarget> {
  if (!desc.color_format && !desc.depth_stencil_format) {
    return nullptr;
  }

  graphics::SharedDeviceChildHandle<graphics::Texture> color_tex;
  graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex;

  if (desc.color_format) {
    D3D12_CLEAR_VALUE const clear_value{.Format = *desc.color_format, .Color = {0.0f, 0.0f, 0.0f, 1.0f}};

    color_tex = device.CreateTexture(graphics::TextureDesc{
      graphics::TextureDimension::k2D, desc.width, desc.height, 1, 1, *desc.color_format,
      DXGI_SAMPLE_DESC{desc.sample_count, 0}, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, false, true, true,
      desc.enable_unordered_access
    }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_RENDER_TARGET, &clear_value);

    std::ignore = color_tex->SetDebugName(desc.debug_name + L" - Color Texture");
  }

  if (desc.depth_stencil_format) {
    D3D12_CLEAR_VALUE const clear_value{.Format = *desc.depth_stencil_format, .DepthStencil = {0.0f, 0}};

    depth_stencil_tex = device.CreateTexture(graphics::TextureDesc{
      graphics::TextureDimension::k2D, desc.width, desc.height, 1, 1, *desc.depth_stencil_format,
      DXGI_SAMPLE_DESC{desc.sample_count, 0}, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL, true, false, true,
      desc.enable_unordered_access
    }, D3D12_HEAP_TYPE_DEFAULT, D3D12_BARRIER_LAYOUT_RENDER_TARGET, &clear_value);

    std::ignore = depth_stencil_tex->SetDebugName(desc.debug_name + L" - Depth-Stencil Texture");
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
  return color_tex_;
}


RenderTarget::RenderTarget(Desc desc, graphics::SharedDeviceChildHandle<graphics::Texture> color_tex,
                           graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex) :
  desc_{std::move(desc)},
  color_tex_{std::move(color_tex)},
  depth_stencil_tex_{std::move(depth_stencil_tex)} {}
}
