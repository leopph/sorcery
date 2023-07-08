#include "SwapChain.hpp"
#include "Systems.hpp"

#include <stdexcept>

#include <dxgi1_5.h>

using Microsoft::WRL::ComPtr;


namespace sorcery {
auto SwapChain::CreateRtv() -> void {
  ComPtr<ID3D11Texture2D> backBuf;
  if (FAILED(mSwapChain->GetBuffer(0, IID_PPV_ARGS(backBuf.GetAddressOf())))) {
    throw std::runtime_error{ "Failed to get swap chain backbuffer." };
  }

  D3D11_RENDER_TARGET_VIEW_DESC constexpr rtvDesc{
    .Format = FORMAT,
    .ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D,
    .Texture2D = { .MipSlice = 0 }
  };

  if (FAILED(mDevice->CreateRenderTargetView(backBuf.Get(), &rtvDesc, mRtv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create swap chain RTV." };
  }
}


SwapChain::SwapChain(ComPtr<ID3D11Device> device, IDXGIFactory2* const factory):
  mDevice{ std::move(device) } {
  if (ComPtr<IDXGIFactory5> factory5; SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(factory5.GetAddressOf())))) {
    if (BOOL allowTearing{ FALSE }; SUCCEEDED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof allowTearing)) && allowTearing) {
      mSwapChainFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
      mPresentFlags |= DXGI_PRESENT_ALLOW_TEARING;
    }
  }

  DXGI_SWAP_CHAIN_DESC1 const desc{
    .Width = 0,
    .Height = 0,
    .Format = FORMAT,
    .Stereo = FALSE,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
    .BufferCount = 2,
    .Scaling = DXGI_SCALING_NONE,
    .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
    .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
    .Flags = mSwapChainFlags
  };

  if (FAILED(factory->CreateSwapChainForHwnd(mDevice.Get(), gWindow.GetHandle(), &desc, nullptr, nullptr, mSwapChain.GetAddressOf()))) {
    throw std::runtime_error{ "Failed to create swap chain." };
  }

  CreateRtv();
}


auto SwapChain::Present(UINT const syncInterval) const -> void {
  if (FAILED(mSwapChain->Present(syncInterval, mPresentFlags))) {
    throw std::runtime_error{ "Failed to present swap chain." };
  }
}


auto SwapChain::Resize(UINT const width, UINT const height) -> void {
  if (width == 0 || height == 0) {
    return;
  }

  mRtv.Reset();

  if (FAILED(mSwapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, mSwapChainFlags))) {
    throw std::runtime_error{ "Failed to resize swap chain buffers." };
  }

  CreateRtv();
}


auto SwapChain::GetRtv() const noexcept -> ID3D11RenderTargetView* {
  return mRtv.Get();
}
}
