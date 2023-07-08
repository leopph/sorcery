#include "RenderTarget.hpp"

#include <stdexcept>


namespace sorcery {
auto RenderTarget::Recreate() -> void {
  D3D11_TEXTURE2D_DESC const hdrTexDesc{
    .Width = mWidth,
    .Height = mHeight,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_R16G16B16A16_FLOAT,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  if (FAILED(mDevice->CreateTexture2D(&hdrTexDesc, nullptr, mHdrTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target HDR texture." };
  }

  D3D11_RENDER_TARGET_VIEW_DESC const hdrRtvDesc{
    .Format = hdrTexDesc.Format,
    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
    .Texture2D
    {
      .MipSlice = 0
    }
  };

  if (FAILED(mDevice->CreateRenderTargetView(mHdrTex.Get(), &hdrRtvDesc, mHdrRtv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target HDR RTV." };
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC const hdrSrvDesc{
    .Format = hdrTexDesc.Format,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D =
    {
      .MostDetailedMip = 0,
      .MipLevels = 1
    }
  };

  if (FAILED(mDevice->CreateShaderResourceView(mHdrTex.Get(), &hdrSrvDesc, mHdrSrv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target HDR SRV." };
  }

  D3D11_TEXTURE2D_DESC const outputTexDesc{
    .Width = mWidth,
    .Height = mHeight,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc
    {
      .Count = 1,
      .Quality = 0
    },
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  if (FAILED(mDevice->CreateTexture2D(&outputTexDesc, nullptr, mOutTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target output texture." };
  }

  D3D11_RENDER_TARGET_VIEW_DESC const outputRtvDesc{
    .Format = outputTexDesc.Format,
    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
    .Texture2D
    {
      .MipSlice = 0
    }
  };

  if (FAILED(mDevice->CreateRenderTargetView(mOutTex.Get(), &outputRtvDesc, mOutRtv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target output RTV." };
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC const outputSrvDesc{
    .Format = outputTexDesc.Format,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D =
    {
      .MostDetailedMip = 0,
      .MipLevels = 1
    }
  };

  if (FAILED(mDevice->CreateShaderResourceView(mOutTex.Get(), &outputSrvDesc, mOutSrv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target output SRV." };
  }

  D3D11_TEXTURE2D_DESC const dsTexDesc{
    .Width = mWidth,
    .Height = mHeight,
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_D24_UNORM_S8_UINT,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_DEPTH_STENCIL,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  if (FAILED(mDevice->CreateTexture2D(&dsTexDesc, nullptr, mDepthTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target depth-stencil texture." };
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC const dsDsvDesc{
    .Format = dsTexDesc.Format,
    .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
    .Flags = 0,
    .Texture2D = { .MipSlice = 0 }
  };

  if (FAILED(mDevice->CreateDepthStencilView(mDepthTex.Get(), &dsDsvDesc,mDsv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to recreate Render Target DSV." };
  }
}


RenderTarget::RenderTarget(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT const width, UINT const height):
  mDevice{ std::move(device) },
  mWidth{ width },
  mHeight{ height } {
  Recreate();
}


auto RenderTarget::Resize(UINT const width, UINT const height) -> void {
  mWidth = width;
  mHeight = height;
  Recreate();
}


auto RenderTarget::GetHdrRtv() const noexcept -> ID3D11RenderTargetView* {
  return mHdrRtv.Get();
}


auto RenderTarget::GetOutRtv() const noexcept -> ID3D11RenderTargetView* {
  return mOutRtv.Get();
}


auto RenderTarget::GetHdrSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mHdrSrv.Get();
}


auto RenderTarget::GetOutSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mOutSrv.Get();
}


auto RenderTarget::GetDsv() const noexcept -> ID3D11DepthStencilView* {
  return mDsv.Get();
}


auto RenderTarget::GetWidth() const noexcept -> UINT {
  return mWidth;
}


auto RenderTarget::GetHeight() const noexcept -> UINT {
  return mHeight;
}
}
