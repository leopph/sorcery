#include "RenderTarget.hpp"

#include <stdexcept>

#include "Renderer.hpp"


namespace sorcery {
auto RenderTarget::Desc::operator==(Desc const& other) const -> bool {
  return width == other.width && height == other.height && colorFormat == other.colorFormat && depthBufferBitCount == other.depthBufferBitCount && stencilBufferBitCount == other.stencilBufferBitCount && sampleCount == other.sampleCount && enableUnorderedAccess == other.enableUnorderedAccess;
}


RenderTarget::RenderTarget(Desc const& desc) :
  mDesc{desc} {
  auto const device{gRenderer.GetDevice()};

  if (!desc.colorFormat && desc.depthBufferBitCount == 0 && desc.stencilBufferBitCount == 0) {
    throw std::runtime_error{"Failed to create RenderTarget: descriptor must contain at least a color format or a depth-stencil bit count."};
  }

  if (desc.colorFormat) {
    D3D11_TEXTURE2D_DESC const colorDesc{
      .Width = desc.width,
      .Height = desc.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = *desc.colorFormat,
      .SampleDesc = {.Count = static_cast<UINT>(desc.sampleCount), .Quality = 0},
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | (desc.enableUnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0u),
      .CPUAccessFlags = 0,
      .MiscFlags = 0
    };

    if (FAILED(device->CreateTexture2D(&colorDesc, nullptr, mColorTex.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to create RenderTarget color texture."};
    }

    std::string colorTexName{desc.debugName + " - Color Texture"};
    if (FAILED(mColorTex->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(colorTexName)), colorTexName.data()))) {
      throw std::runtime_error{"Failed to set RenderTarget color texture debug name."};
    }

    auto const rtvDesc{
      [&colorDesc] {
        if (colorDesc.SampleDesc.Count > 1) {
          return D3D11_RENDER_TARGET_VIEW_DESC{
            .Format = colorDesc.Format,
            .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS,
            .Texture2DMS = {}
          };
        }

        return D3D11_RENDER_TARGET_VIEW_DESC{
          .Format = colorDesc.Format,
          .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
          .Texture2D = {.MipSlice = 0}
        };
      }()
    };

    if (FAILED(device->CreateRenderTargetView(mColorTex.Get(), &rtvDesc, mRtv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to create RenderTarget RTV."};
    }

    std::string colorRtvName{desc.debugName + " - RTV"};
    if (FAILED(mRtv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(colorRtvName)), colorRtvName.data()))) {
      throw std::runtime_error{"Failed to set RenderTarget RTV debug name."};
    }

    auto const colorSrvDesc{
      [&colorDesc] {
        if (colorDesc.SampleDesc.Count > 1) {
          return D3D11_SHADER_RESOURCE_VIEW_DESC{
            .Format = colorDesc.Format,
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS,
            .Texture2DMS = {}
          };
        }

        return D3D11_SHADER_RESOURCE_VIEW_DESC{
          .Format = colorDesc.Format,
          .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
          .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
        };
      }()
    };

    if (FAILED(device->CreateShaderResourceView(mColorTex.Get(), &colorSrvDesc, mColorSrv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to create RenderTarget color SRV."};
    }

    std::string colorSrvName{desc.debugName + " - Color SRV"};
    if (FAILED(mColorSrv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(colorSrvName)), colorSrvName.data()))) {
      throw std::runtime_error{"Failed to set RenderTarget color SRV debug name."};
    }

    if (desc.enableUnorderedAccess && colorDesc.SampleDesc.Count == 1) {
      D3D11_UNORDERED_ACCESS_VIEW_DESC const colorUavDesc{
        .Format = colorDesc.Format,
        .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
        .Texture2D = {.MipSlice = 0}
      };

      if (FAILED(device->CreateUnorderedAccessView(mColorTex.Get(), &colorUavDesc, mColorUav.GetAddressOf()))) {
        throw std::runtime_error{"Failed to create RenderTarget color UAV."};
      }

      std::string colorUavName{desc.debugName + " - Color UAV"};
      if (FAILED(mColorUav->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(colorUavName)), colorUavName.data()))) {
        throw std::runtime_error{"Failed to set RenderTarget color UAV debug name."};
      }
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
        throw std::runtime_error{"Unsupported depth-stencil buffer configuration."};
      }()
    };

    D3D11_TEXTURE2D_DESC const depthStencilDesc{
      .Width = desc.width,
      .Height = desc.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = textureFormat,
      .SampleDesc = {.Count = static_cast<UINT>(desc.sampleCount), .Quality = 0},
      .Usage = D3D11_USAGE_DEFAULT,
      .BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE | (desc.enableUnorderedAccess ? D3D11_BIND_UNORDERED_ACCESS : 0u),
      .CPUAccessFlags = 0,
      .MiscFlags = 0
    };

    if (FAILED(device->CreateTexture2D(&depthStencilDesc, nullptr, mDepthStencilTex.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to create RenderTarget depth-stencil texture."};
    }

    std::string depthStencilTexName{desc.debugName + " - Depth-Stencil Texture"};
    if (FAILED(mDepthStencilTex->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(depthStencilTexName)), depthStencilTexName.data()))) {
      throw std::runtime_error{"Failed to set RenderTarget depth-stencil texture debug name."};
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
            throw std::runtime_error{"Unsupported depth-stencil buffer configuration."};
        }
      }()
    };

    auto const dsvDesc{
      [&depthStencilDesc, dsvFormat] {
        if (depthStencilDesc.SampleDesc.Count > 1) {
          return D3D11_DEPTH_STENCIL_VIEW_DESC{
            .Format = dsvFormat,
            .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS,
            .Flags = 0,
            .Texture2DMS = {}
          };
        }

        return D3D11_DEPTH_STENCIL_VIEW_DESC{
          .Format = dsvFormat,
          .ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D,
          .Flags = 0,
          .Texture2D = {.MipSlice = 0}
        };
      }()
    };

    if (FAILED(device->CreateDepthStencilView(mDepthStencilTex.Get(), &dsvDesc, mDsv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{"Failed to create RenderTarget DSV."};
    }

    std::string dsvName{desc.debugName + " - DSV"};
    if (FAILED(mDsv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(dsvName)), dsvName.data()))) {
      throw std::runtime_error{"Failed to set RenderTarget DSV debug name."};
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

      auto const depthSrvDesc{
        [&depthStencilDesc, srvFormat] {
          if (depthStencilDesc.SampleDesc.Count > 1) {
            return D3D11_SHADER_RESOURCE_VIEW_DESC{
              .Format = srvFormat,
              .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS,
              .Texture2DMS = {}
            };
          }

          return D3D11_SHADER_RESOURCE_VIEW_DESC{
            .Format = srvFormat,
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
          };
        }()
      };

      if (FAILED(device->CreateShaderResourceView(mDepthStencilTex.Get(), &depthSrvDesc, mDepthSrv.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error{"Failed to create RenderTarget depth SRV."};
      }

      std::string depthSrvName{desc.debugName + " - Depth SRV"};
      if (FAILED(mDepthSrv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(depthSrvName)), depthSrvName.data()))) {
        throw std::runtime_error{"Failed to set RenderTarget depth SRV debug name."};
      }

      if (desc.enableUnorderedAccess && depthStencilDesc.SampleDesc.Count == 1) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC const depthUavDesc{
          .Format = depthSrvDesc.Format,
          .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
          .Texture2D = {.MipSlice = 0}
        };

        if (FAILED(device->CreateUnorderedAccessView(mDepthStencilTex.Get(), &depthUavDesc, mDepthUav.GetAddressOf()))) {
          throw std::runtime_error{"Failed to create RenderTarget depth UAV."};
        }

        std::string depthUavName{desc.debugName + " - Depth UAV"};
        if (FAILED(mDepthUav->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(depthUavName)), depthUavName.data()))) {
          throw std::runtime_error{"Failed to set RenderTarget depth UAV debug name."};
        }
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

      auto const stencilSrvDesc{
        [&depthStencilDesc, srvFormat] {
          if (depthStencilDesc.SampleDesc.Count > 1) {
            return D3D11_SHADER_RESOURCE_VIEW_DESC{
              .Format = srvFormat,
              .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS,
              .Texture2DMS = {}
            };
          }

          return D3D11_SHADER_RESOURCE_VIEW_DESC{
            .Format = srvFormat,
            .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
            .Texture2D = {.MostDetailedMip = 0, .MipLevels = 1}
          };
        }()
      };

      if (FAILED(device->CreateShaderResourceView(mDepthStencilTex.Get(), &stencilSrvDesc, mStencilSrv.ReleaseAndGetAddressOf()))) {
        throw std::runtime_error{"Failed to create RenderTarget stencil SRV."};
      }

      std::string stencilSrvName{desc.debugName + " - Stencil SRV"};
      if (FAILED(mStencilSrv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(stencilSrvName)), stencilSrvName.data()))) {
        throw std::runtime_error{"Failed to set RenderTarget stencil SRV debug name."};
      }

      if (desc.enableUnorderedAccess && depthStencilDesc.SampleDesc.Count == 1) {
        D3D11_UNORDERED_ACCESS_VIEW_DESC const stencilUavDesc{
          .Format = stencilSrvDesc.Format,
          .ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D,
          .Texture2D = {.MipSlice = 0}
        };

        if (FAILED(device->CreateUnorderedAccessView(mDepthStencilTex.Get(), &stencilUavDesc, mStencilUav.GetAddressOf()))) {
          throw std::runtime_error{"Failed to create RenderTarget stencil UAV."};
        }

        std::string stencilUavName{desc.debugName + " - Stencil UAV"};
        if (FAILED(mStencilUav->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(std::size(stencilUavName)), stencilUavName.data()))) {
          throw std::runtime_error{"Failed to set RenderTarget stencil UAV debug name."};
        }
      }
    }
  }
}


auto RenderTarget::GetDesc() const noexcept -> Desc const& {
  return mDesc;
}


auto RenderTarget::GetColorTexture() const noexcept -> ObserverPtr<ID3D11Texture2D> {
  return mColorTex.Get();
}


auto RenderTarget::GetDepthStencilTexture() const noexcept -> ObserverPtr<ID3D11Texture2D> {
  return mDepthStencilTex.Get();
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


auto RenderTarget::GetColorUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView> {
  return mColorUav.Get();
}


auto RenderTarget::GetDepthUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView> {
  return mDepthUav.Get();
}


auto RenderTarget::GetStencilUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView> {
  return mStencilUav.Get();
}
}
