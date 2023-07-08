#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace sorcery {
class RenderTarget {
  Microsoft::WRL::ComPtr<ID3D11Device> mDevice;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> mHdrTex;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mHdrRtv;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mHdrSrv;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> mOutTex;
  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mOutRtv;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mOutSrv;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthTex;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDsv;

  UINT mWidth;
  UINT mHeight;

  auto Recreate() -> void;

public:
  RenderTarget(Microsoft::WRL::ComPtr<ID3D11Device> device, UINT width, UINT height);

  auto Resize(UINT width, UINT height) -> void;

  [[nodiscard]] auto GetHdrRtv() const noexcept -> ID3D11RenderTargetView*;
  [[nodiscard]] auto GetOutRtv() const noexcept -> ID3D11RenderTargetView*;
  [[nodiscard]] auto GetHdrSrv() const noexcept -> ID3D11ShaderResourceView*;
  [[nodiscard]] auto GetOutSrv() const noexcept -> ID3D11ShaderResourceView*;
  [[nodiscard]] auto GetDsv() const noexcept -> ID3D11DepthStencilView*;

  [[nodiscard]] auto GetWidth() const noexcept -> UINT;
  [[nodiscard]] auto GetHeight() const noexcept -> UINT;
};
}
