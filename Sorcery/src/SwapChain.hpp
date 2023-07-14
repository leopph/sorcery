#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>


namespace sorcery {
class SwapChain {
  DXGI_FORMAT static constexpr COLOR_FORMAT{ DXGI_FORMAT_R8G8B8A8_UNORM };
  DXGI_FORMAT static constexpr DEPTH_STENCIL_FORMAT{ DXGI_FORMAT_D24_UNORM_S8_UINT };

  Microsoft::WRL::ComPtr<ID3D11Device> mDevice;

  Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRtv;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilTex;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDsv;

  UINT mSwapChainFlags{ 0 };
  UINT mPresentFlags{ 0 };

  auto RecreateViews() -> void;
  auto RecreateDepthStencilTex(UINT width, UINT height) -> void;

public:
  SwapChain(Microsoft::WRL::ComPtr<ID3D11Device> device, IDXGIFactory2* factory);

  auto Present(UINT syncInterval) const -> void;
  auto Resize(UINT width, UINT height) -> void;

  [[nodiscard]] auto GetRtv() const noexcept -> ID3D11RenderTargetView*;
  [[nodiscard]] auto GetDsv() const noexcept -> ID3D11DepthStencilView*;
};
}
