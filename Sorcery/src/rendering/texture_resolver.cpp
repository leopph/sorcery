#include "texture_resolver.hpp"

#include <cassert>

#include "../resources/Cubemap.hpp"
#include "../resources/Texture2D.hpp"


namespace sorcery::rendering {
TextureResolver::TextureResolver(wand::GraphicsDevice& device, RenderResourceRegistry& registry) :
  device_{&device},
  registry_{&registry} {}


auto TextureResolver::Resolve(Texture2D& tex, RenderFrame& frame) const -> wand::SharedDeviceHandle<wand::Texture> {
  auto const [render_tex, is_new] = registry_->CreateOrGetTexture(tex.GetId());

  if (is_new) {
    auto const img = tex.GetData();
    assert(img);

    Upload(*img, *render_tex, frame);

    if (tex.GetCpuDataPolicy() == CpuResidencyPolicy::kReleaseAfterUpload) {
      detail::ClearTex2DCpuData(tex);
    }
  }

  return render_tex->GetTexture();
}


auto TextureResolver::Resolve(Cubemap& cubemap, RenderFrame& frame) const -> wand::SharedDeviceHandle<wand::Texture> {
  auto const [render_tex, is_new] = registry_->CreateOrGetTexture(cubemap.GetId());

  if (is_new) {
    auto const img = cubemap.GetData();
    assert(img);

    Upload(*img, *render_tex, frame);

    if (cubemap.GetCpuDataPolicy() == CpuResidencyPolicy::kReleaseAfterUpload) {
      detail::ClearCubemapCpuData(cubemap);
    }
  }

  return render_tex->GetTexture();
}


auto TextureResolver::Upload(DirectX::ScratchImage const& img, RenderTexture& render_tex,
                             RenderFrame& frame) const -> void {
  render_tex.Init(*device_, img.GetMetadata());

  std::vector<D3D12_SUBRESOURCE_DATA> subresource_data;
  subresource_data.reserve(img.GetImageCount());

  for (auto i = 0uz; i < img.GetImageCount(); ++i) {
    auto const subimg = img.GetImages()[i];

    subresource_data.emplace_back(subimg.pixels, static_cast<LONG_PTR>(subimg.rowPitch),
      static_cast<LONG_PTR>(subimg.slicePitch));
  }

  frame.UploadTexture(render_tex.GetTexture(), 0, subresource_data);
}
}
