#include "DirectionalShadowMapArray.hpp"


#include <cassert>


namespace sorcery {
DirectionalShadowMapArray::DirectionalShadowMapArray(ID3D11Device* const device, int const size) :
  mSize{size} {
  D3D11_TEXTURE2D_DESC const texDesc{
    .Width = static_cast<UINT>(size),
    .Height = static_cast<UINT>(size),
    .MipLevels = 1,
    .ArraySize = MAX_CASCADE_COUNT,
    .Format = DXGI_FORMAT_R32_TYPELESS,
    .SampleDesc = {.Count = 1, .Quality = 0},
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  [[maybe_unused]] auto hr{device->CreateTexture2D(&texDesc, nullptr, mTex.GetAddressOf())};
  assert(SUCCEEDED(hr));

  for (auto i{0}; i < MAX_CASCADE_COUNT; i++) {
    D3D11_DEPTH_STENCIL_VIEW_DESC const dsvDesc{
      .Format = DXGI_FORMAT_D32_FLOAT,
      .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
      .Flags = 0,
      .Texture2DArray = {.MipSlice = 0, .FirstArraySlice = static_cast<UINT>(i), .ArraySize = 1}
    };

    hr = device->CreateDepthStencilView(mTex.Get(), &dsvDesc, mDsv[i].GetAddressOf());
    assert(SUCCEEDED(hr));
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr srvDesc{
    .Format = DXGI_FORMAT_R32_FLOAT,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
    .Texture2DArray = {.MostDetailedMip = 0, .MipLevels = 1, .FirstArraySlice = 0, .ArraySize = MAX_CASCADE_COUNT}
  };

  hr = device->CreateShaderResourceView(mTex.Get(), &srvDesc, mSrv.GetAddressOf());
  assert(SUCCEEDED(hr));
}


auto DirectionalShadowMapArray::GetDsv(int const idx) const noexcept -> ObserverPtr<ID3D11DepthStencilView> {
  assert(idx >= 0 && idx < MAX_CASCADE_COUNT);
  return mDsv[idx].Get();
}


auto DirectionalShadowMapArray::GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView> {
  return mSrv.Get();
}


auto DirectionalShadowMapArray::GetSize() const noexcept -> int {
  return mSize;
}
}
