#include "sorcery/host/rendering/texture_resolver.hpp"

#include <cassert>

#include "rendering/render_resource_registry.hpp"
#include "sorcery/host/rendering/upload_arena.hpp"
#include "sorcery/assets/Cubemap.hpp"
#include "sorcery/assets/Texture2D.hpp"


namespace sorcery::rendering {
TextureResolver::TextureResolver(wand::GraphicsDevice& device, RenderResourceRegistry& registry) :
  device_{&device},
  registry_{&registry} {}


auto TextureResolver::Resolve(
  Texture2D& tex,
  UploadArena& upload_arena,
  wand::CommandList& cmd
) const -> wand::SharedDeviceHandle<wand::Texture> {
  auto const [render_tex, is_new] = registry_->CreateOrGetTexture(tex.GetId());

  if (is_new) {
    auto const img = tex.GetData();
    assert(img);

    Upload(*img, *render_tex, upload_arena, cmd);

    if (tex.GetCpuDataPolicy() == CpuResidencyPolicy::kReleaseAfterUpload) {
      detail::ClearTex2DCpuData(tex);
    }
  }

  return render_tex->GetTexture();
}


auto TextureResolver::Resolve(
  Cubemap& cubemap,
  UploadArena& upload_arena,
  wand::CommandList& cmd
) const -> wand::SharedDeviceHandle<wand::Texture> {
  auto const [render_tex, is_new] = registry_->CreateOrGetTexture(cubemap.GetId());

  if (is_new) {
    auto const img = cubemap.GetData();
    assert(img);

    Upload(*img, *render_tex, upload_arena, cmd);

    if (cubemap.GetCpuDataPolicy() == CpuResidencyPolicy::kReleaseAfterUpload) {
      detail::ClearCubemapCpuData(cubemap);
    }
  }

  return render_tex->GetTexture();
}


auto TextureResolver::Upload(
  DirectX::ScratchImage const& img,
  RenderTexture& render_tex,
  UploadArena& upload_arena,
  wand::CommandList& cmd
) const -> void {
  render_tex.Init(*device_, img.GetMetadata());

  std::vector<D3D12_SUBRESOURCE_DATA> subresource_data;
  subresource_data.reserve(img.GetImageCount());

  for (auto i = 0uz; i < img.GetImageCount(); ++i) {
    auto const subimg = img.GetImages()[i];

    subresource_data.emplace_back(subimg.pixels, static_cast<LONG_PTR>(subimg.rowPitch),
      static_cast<LONG_PTR>(subimg.slicePitch));
  }

  upload_arena.UploadTexture(cmd, render_tex.GetTexture(), 0, subresource_data);
  cmd.Seal(*render_tex.GetTexture());
}
}
