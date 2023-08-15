#pragma once

#include "Resource.hpp"
#include "../Image.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace sorcery {
class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;

public:
  LEOPPHAPI Cubemap(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;
};
}
