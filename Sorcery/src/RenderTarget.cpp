#include "RenderTarget.hpp"

#include "Systems.hpp"

#include <stdexcept>

#include "Renderer.hpp"


namespace sorcery {
auto RenderTarget::Desc::operator==(Desc const& other) const -> bool {
  return width == other.width && height == other.height && colorFormat == other.colorFormat && depthBufferBitCount == other.depthBufferBitCount && stencilBufferBitCount == other.stencilBufferBitCount;
}


RenderTarget::RenderTarget(Desc const& desc) :
  mDesc{ desc } {
  auto const device{ gRenderer.GetDevice() };

  if (!desc.colorFormat && desc.depthBufferBitCount == 0 && desc.stencilBufferBitCount == 0) {
    throw std::runtime_error{ "Failed to create RenderTarget: descriptor must contain at least a color format or a depth-stencil bit count." };
  }

  if (desc.colorFormat) {
    D3D11_TEXTURE2D_DESC const colorDesc{
      .Width = desc.width,
      .Height = desc.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = *desc.colorFormat,
      .SampleDesc = { .Count = 1, .Quality = 0 },
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
      .CPUAccessFlags = 0,
      .MiscFlags = 0
    };

    if (FAILED(device->CreateTexture2D(&colorDesc, nullptr, mColorTex.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget color texture." };
    }

    std::string colorTexName{ desc.debugName + " - Color Texture" };
    if (FAILED(mColorTex->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(colorTexName), colorTexName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget color texture debug name." };
    }

    D3D11_RENDER_TARGET_VIEW_DESC const rtvDesc{
      .Format = colorDesc.Format,
      .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
      .Texture2D = { .MipSlice = 0 }
    };

    if (FAILED(device->CreateRenderTargetView(mColorTex.Get(), &rtvDesc, mRtv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget RTV." };
    }

    std::string colorRtvName{ desc.debugName + " - RTV" };
    if (FAILED(mRtv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(colorRtvName), colorRtvName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget RTV debug name." };
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC const colorSrvDesc{
      .Format = colorDesc.Format,
      .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
      .Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
    };

    if (FAILED(device->CreateShaderResourceView(mColorTex.Get(), &colorSrvDesc, mColorSrv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget color SRV." };
    }

    std::string colorSrvName{ desc.debugName + " - Color SRV" };
    if (FAILED(mColorSrv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(colorSrvName), colorSrvName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget color SRV debug name." };
    }
  }

  if (desc.depthBufferBitCount > 0 || desc.stencilBufferBitCount > 0) {
    auto const textureFormat{
      [&desc] {
        if (desc.depthBufferBitCount == 16 && desc.stencilBufferBitCount == 0) {
          return DXGI_FORMAT_R16_TYPELESS;
        }
        if (desc.depthBufferBitCount == 24 && (desc.stencilBufferBitCount == 0 || desc.stencilBufferBitCount == 8)) {
          return DXGI_FORMAT_R24G8_TYPELESS;
        }
        if (desc.depthBufferBitCount == 32 && desc.stencilBufferBitCount == 0) {
          return DXGI_FORMAT_R32_TYPELESS;
        }
        if (desc.depthBufferBitCount == 32 && desc.stencilBufferBitCount == 0) {
          return DXGI_FORMAT_R32G8X24_TYPELESS;
        }
        throw std::runtime_error{ "Unsupported depth-stencil buffer configuration." };
      }()
    };

    D3D11_TEXTURE2D_DESC const depthStencilDesc{
      .Width = desc.width,
      .Height = desc.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = textureFormat,
      .SampleDesc = { .Count = 1, .Quality = 0 },
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE,
      .CPUAccessFlags = 0,
      .MiscFlags = 0
    };

    if (FAILED(device->CreateTexture2D(&depthStencilDesc, nullptr, mDepthStencilTex.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget depth-stencil texture." };
    }

    std::string depthStencilTexName{ desc.debugName + " - Depth-Stencil Texture" };
    if (FAILED(mDepthStencilTex->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(depthStencilTexName), depthStencilTexName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget depth-stencil texture debug name." };
    }

    auto const dsvFormat{
      [textureFormat] {
        switch (textureFormat) {
          case DXGI_FORMAT_R16_TYPELESS:
            return DXGI_FORMAT_D16_UNORM;
          case DXGI_FORMAT_R24G8_TYPELESS:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
          case DXGI_FORMAT_R32_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT;
          case DXGI_FORMAT_R32G8X24_TYPELESS:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
          default:
            throw std::runtime_error{ "Unsupported depth-stencil buffer configuration." };
        }
      }()
    };

    D3D11_DEPTH_STENCIL_VIEW_DESC const dsvDesc
    {
      .Format = dsvFormat,
      .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
      .Flags = 0,
      .Texture2D = { .MipSlice = 0 }
    };

    if (FAILED(device->CreateDepthStencilView(mDepthStencilTex.Get(), &dsvDesc, mDsv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget DSV." };
    }

    std::string dsvName{ desc.debugName + " - DSV" };
    if (FAILED(mDsv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(dsvName), dsvName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget DSV debug name." };
    }

    if (desc.depthBufferBitCount > 0) {
      auto const srvFormat{
        [&desc] {
          return desc.depthBufferBitCount == 16
                   ? DXGI_FORMAT_R16_UNORM
                   : desc.depthBufferBitCount == 24
                       ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS
                       : desc.stencilBufferBitCount == 0
                           ? DXGI_FORMAT_R32_FLOAT
                           : DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        }()
      };

      D3D11_SHADER_RESOURCE_VIEW_DESC const depthSrvDesc{
        .Format = srvFormat,
        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        .Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
      };

      if (FAILED(device->CreateShaderResourceView(mDepthStencilTex.Get(), &depthSrvDesc, mDepthSrv.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error{ "Failed to create RenderTarget depth SRV." };
      }

      std::string depthSrvName{ desc.debugName + " - Depth SRV" };
      if (FAILED(mDepthSrv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(depthSrvName), depthSrvName.data()))) {
        throw std::runtime_error{ "Failed to set RenderTarget depth SRV debug name." };
      }
    }

    if (desc.stencilBufferBitCount > 0) {
      auto const srvFormat{
        [&desc] {
          return desc.depthBufferBitCount == 24
                   ? DXGI_FORMAT_X24_TYPELESS_G8_UINT
                   : DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;
        }()
      };

      D3D11_SHADER_RESOURCE_VIEW_DESC const stencilSrvDesc{
        .Format = srvFormat,
        .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
        .Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
      };

      if (FAILED(device->CreateShaderResourceView(mDepthStencilTex.Get(), &stencilSrvDesc, mStencilSrv.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error{ "Failed to create RenderTarget stencil SRV." };
      }

      std::string stencilSrvName{ desc.debugName + " - Stencil SRV" };
      if (FAILED(mStencilSrv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(stencilSrvName), stencilSrvName.data()))) {
        throw std::runtime_error{ "Failed to set RenderTarget stencil SRV debug name." };
      }
    }
  }
}


auto RenderTarget::GetDesc() const noexcept -> Desc const& {
  return mDesc;
}


auto RenderTarget::GetRtv() const noexcept -> ID3D11RenderTargetView* {
  return mRtv.Get();
}


auto RenderTarget::GetDsv() const noexcept -> ID3D11DepthStencilView* {
  return mDsv.Get();
}


auto RenderTarget::GetColorSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mColorSrv.Get();
}


auto RenderTarget::GetDepthSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mDepthSrv.Get();
}


auto RenderTarget::GetStencilSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mStencilSrv.Get();
}
}
