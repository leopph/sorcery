#include "Cubemap.hpp"

#include "Renderer.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<leopph::Cubemap>{ "Cubemap" };
}


namespace leopph {
auto Cubemap::UploadToGpu() -> void {
  D3D11_TEXTURE2D_DESC const texDesc{
    .Width = clamp_cast<UINT>(mFaceData[0].get_width()),
    .Height = clamp_cast<UINT>(mFaceData[0].get_height()),
    .MipLevels = 1,
    .ArraySize = 6,
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE
  };

  std::array<D3D11_SUBRESOURCE_DATA, 6> texData;

  for (int i = 0; i < 6; i++) {
    texData[i] = D3D11_SUBRESOURCE_DATA{
      .pSysMem = mFaceData[i].get_data().data(),
      .SysMemPitch = mFaceData[i].get_width() * mFaceData[i].get_num_channels(),
      .SysMemSlicePitch = 0
    };
  }

  if (FAILED(renderer::GetDevice()->CreateTexture2D(&texDesc, texData.data(), mTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create GPU texture of Cubemap." };
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr srvDesc{
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE,
    .TextureCube = {
      .MostDetailedMip = 0,
      .MipLevels = 1
    }
  };

  if (FAILED(renderer::GetDevice()->CreateShaderResourceView(mTex.Get(), &srvDesc, mSrv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create GPU SRV of Cubemap." };
  }
}


auto Cubemap::GetSerializationType() const -> Type {
  return SerializationType;
}


Cubemap::Cubemap(std::span<Image, 6> faces) {
  std::ranges::move(faces, std::begin(mTmpFaceData));
  Update();
}


auto Cubemap::Update() noexcept -> void {
  mFaceData = std::move(mTmpFaceData);
  UploadToGpu();
}


auto Cubemap::GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView> {
  return mSrv.Get();
}
}
