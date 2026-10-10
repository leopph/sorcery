#include "render_texture.hpp"


namespace sorcery::rendering {
auto RenderTexture::Init(wand::GraphicsDevice& device, DirectX::TexMetadata const& metadata) -> void {
  wand::TextureDesc desc;
  desc.width = static_cast<UINT>(metadata.width);
  desc.mip_levels = static_cast<UINT16>(metadata.mipLevels);
  desc.format = metadata.format;
  desc.sample_count = 1;
  desc.depth_stencil = false;
  desc.render_target = false;
  desc.shader_resource = true;
  desc.unordered_access = false;

  if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE1D) {
    desc.dimension = wand::TextureDimension::k1D;
    desc.height = 1;
    desc.depth_or_array_size = static_cast<UINT16>(metadata.arraySize);
  } else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE2D) {
    if (metadata.IsCubemap()) {
      desc.dimension = wand::TextureDimension::kCube;
    } else {
      desc.dimension = wand::TextureDimension::k2D;
    }
    desc.height = static_cast<UINT>(metadata.height);
    desc.depth_or_array_size = static_cast<UINT16>(metadata.arraySize);
  } else if (metadata.dimension == DirectX::TEX_DIMENSION_TEXTURE3D) {
    desc.dimension = wand::TextureDimension::k3D;
    desc.height = static_cast<UINT>(metadata.height);
    desc.depth_or_array_size = static_cast<UINT16>(metadata.depth);
  }

  tex_ = device.CreateTexture(desc, wand::CpuAccess::kNone, nullptr);
}


auto RenderTexture::GetTexture() const -> wand::SharedDeviceHandle<wand::Texture> const& {
  return tex_;
}
}
