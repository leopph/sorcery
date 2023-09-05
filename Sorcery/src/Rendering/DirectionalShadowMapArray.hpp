#pragma once

#include "../Core.hpp"
#include "../shaders/ShaderInterop.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include <array>


namespace sorcery {
class DirectionalShadowMapArray {
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, MAX_CASCADE_COUNT> mDsv;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  int mSize;

public:
  explicit DirectionalShadowMapArray(ID3D11Device* device, int size);

  [[nodiscard]] auto GetDsv(int idx) const noexcept -> ObserverPtr<ID3D11DepthStencilView>;
  [[nodiscard]] auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;
  [[nodiscard]] auto GetSize() const noexcept -> int;
};
}
