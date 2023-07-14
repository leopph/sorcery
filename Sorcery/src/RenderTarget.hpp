#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <optional>
#include <string>

#include "Core.hpp"


namespace sorcery {
class RenderTarget {
public:
  struct Desc {
    UINT width{ 1024 };
    UINT height{ 1024 };

    std::optional<DXGI_FORMAT> colorFormat{ DXGI_FORMAT_R8G8B8A8_UNORM };
    int depthBufferBitCount;
    int stencilBufferBitCount;

    std::string debugName;

    LEOPPHAPI [[nodiscard]] auto operator==(Desc const& other) const -> bool;
  };

private:
  Desc mDesc;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> mColorTex;
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mDepthStencilTex;

  Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRtv;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDsv;

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorSrv;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mDepthSrv;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mStencilSrv;

public:
  LEOPPHAPI explicit RenderTarget(Desc const& desc);
  RenderTarget(RenderTarget const&) = delete;
  RenderTarget(RenderTarget&&) = delete;

  ~RenderTarget() = default;

  auto operator=(RenderTarget const&) -> void = delete;
  auto operator=(RenderTarget&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetDesc() const noexcept -> Desc const&;

  [[nodiscard]] LEOPPHAPI auto GetRtv() const noexcept -> ID3D11RenderTargetView*;
  [[nodiscard]] LEOPPHAPI auto GetDsv() const noexcept -> ID3D11DepthStencilView*;

  [[nodiscard]] LEOPPHAPI auto GetColorSrv() const noexcept -> ID3D11ShaderResourceView*;
  [[nodiscard]] LEOPPHAPI auto GetDepthSrv() const noexcept -> ID3D11ShaderResourceView*;
  [[nodiscard]] LEOPPHAPI auto GetStencilSrv() const noexcept -> ID3D11ShaderResourceView*;
};
}
