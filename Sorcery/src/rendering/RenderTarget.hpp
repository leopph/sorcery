#pragma once

#include "../graphics_platform.hpp"

#include <optional>
#include <string>

#include "../Core.hpp"


namespace sorcery {
class RenderTarget {
public:
  struct Desc {
    UINT width{1024};
    UINT height{1024};

    std::optional<DXGI_FORMAT> colorFormat{DXGI_FORMAT_R8G8B8A8_UNORM};
    int depthBufferBitCount{0};
    int stencilBufferBitCount{0};

    int sampleCount{1};

    std::string debugName;

    bool enableUnorderedAccess{false};

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

  Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mColorUav;
  Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mDepthUav;
  Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> mStencilUav;

public:
  LEOPPHAPI explicit RenderTarget(Desc const& desc);
  RenderTarget(RenderTarget const&) = delete;
  RenderTarget(RenderTarget&&) = delete;

  ~RenderTarget() = default;

  auto operator=(RenderTarget const&) -> void = delete;
  auto operator=(RenderTarget&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetDesc() const noexcept -> Desc const&;

  [[nodiscard]] LEOPPHAPI auto GetColorTexture() const noexcept -> ObserverPtr<ID3D11Texture2D>;
  [[nodiscard]] LEOPPHAPI auto GetDepthStencilTexture() const noexcept -> ObserverPtr<ID3D11Texture2D>;

  [[nodiscard]] LEOPPHAPI auto GetRtv() const noexcept -> ObserverPtr<ID3D11RenderTargetView>;
  [[nodiscard]] LEOPPHAPI auto GetDsv() const noexcept -> ObserverPtr<ID3D11DepthStencilView>;

  [[nodiscard]] LEOPPHAPI auto GetColorSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;
  [[nodiscard]] LEOPPHAPI auto GetDepthSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;
  [[nodiscard]] LEOPPHAPI auto GetStencilSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;

  [[nodiscard]] LEOPPHAPI auto GetColorUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView>;
  [[nodiscard]] LEOPPHAPI auto GetDepthUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView>;
  [[nodiscard]] LEOPPHAPI auto GetStencilUav() const noexcept -> ObserverPtr<ID3D11UnorderedAccessView>;
};
}
