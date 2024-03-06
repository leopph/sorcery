#pragma once

#include "Resource.hpp"
#include "../Image.hpp"
#include "../Util.hpp"


namespace sorcery {
class Texture2D final : public Resource {
  RTTR_ENABLE(Resource)
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11Resource> mRes;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;

  int mWidth;
  int mHeight;
  int mChannelCount;

public:
  LEOPPHAPI Texture2D(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> NotNull<ObserverPtr<ID3D11ShaderResourceView>>;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> int;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
