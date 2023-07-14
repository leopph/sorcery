#include "RenderTarget.hpp"

#include "Systems.hpp"

#include <stdexcept>

#include "Renderer.hpp"


namespace sorcery {
RenderTarget::RenderTarget(Desc const& desc) :
  mDesc{ desc } {
  auto const device{ gRenderer.GetDevice() };

  if (!desc.colorFormat && !desc.depthStencilFormat) {
    throw std::runtime_error{ "Failed to create RenderTarget: descriptor must contain at least a color format or a depth-stencil format." };
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

  if (desc.depthStencilFormat) {
    D3D11_TEXTURE2D_DESC const depthStencilDesc{
      .Width = desc.width,
      .Height = desc.height,
      .MipLevels = 1,
      .ArraySize = 1,
      .Format = *desc.depthStencilFormat,
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

    D3D11_DEPTH_STENCIL_VIEW_DESC const dsvDesc{
      .Format = depthStencilDesc.Format,
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

    D3D11_SHADER_RESOURCE_VIEW_DESC const depthStencilSrvDesc{
      .Format = depthStencilDesc.Format,
      .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
      .Texture2D = { .MostDetailedMip = 0, .MipLevels = 1 }
    };

    if (FAILED(device->CreateShaderResourceView(mDepthStencilTex.Get(), &depthStencilSrvDesc, mDepthStencilSrv.ReleaseAndGetAddressOf()))) {
      throw std::runtime_error{ "Failed to create RenderTarget depth-stencil SRV." };
    }

    std::string depthStencilSrvName{ desc.debugName + " - Depth-Stencil SRV" };
    if (FAILED(mDepthStencilSrv->SetPrivateData(WKPDID_D3DDebugObjectName, std::ssize(depthStencilSrvName), depthStencilSrvName.data()))) {
      throw std::runtime_error{ "Failed to set RenderTarget depth-stencil SRV debug name." };
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


auto RenderTarget::GetDepthStencilSrv() const noexcept -> ID3D11ShaderResourceView* {
  return mDepthStencilSrv.Get();
}
}
