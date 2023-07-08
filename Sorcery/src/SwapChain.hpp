#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>


namespace sorcery {
class SwapChain {
  DXGI_FORMAT static constexpr FORMAT{ DXGI_FORMAT_R8G8B8A8_UNORM };

  Microsoft::WRL::ComPtr<ID3D11Device> mDevice;
  Microsoft::WRL::ComPtr<IDXGISwapChain1> mSwapChain;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRtv;

  UINT mSwapChainFlags{ 0 };
  UINT mPresentFlags{ 0 };

  auto CreateRtv() -> void;

public:
  SwapChain(Microsoft::WRL::ComPtr<ID3D11Device> device, IDXGIFactory2* factory);

  auto Present(UINT syncInterval) const -> void;
  auto Resize(UINT width, UINT height) -> void;

  [[nodiscard]] auto GetRtv() const noexcept -> ID3D11RenderTargetView*;
};
}
