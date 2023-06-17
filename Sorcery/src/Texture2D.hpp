#pragma once

#include "Object.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include "Image.hpp"
#include "Util.hpp"


namespace sorcery {
class Texture2D final : public Object {
  RTTR_ENABLE(Object)
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  Image mImgData{};
  Image mTmpImgData{};

  auto UploadToGPU() -> void;

public:
  Texture2D() = default;
  LEOPPHAPI explicit Texture2D(Image img);

  [[nodiscard]] LEOPPHAPI auto GetImageData() const noexcept -> Image const&;
  LEOPPHAPI auto SetImageData(Image img) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;

  LEOPPHAPI auto Update() noexcept -> void;

  LEOPPHAPI Type constexpr static SerializationType{ Type::Texture2D };
  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
};
}
