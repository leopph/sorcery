#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include "Object.hpp"
#include "Image.hpp"
#include "Util.hpp"

#include <array>


namespace leopph {
class Cubemap : public Object {
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  std::array<Image, 6> mFaceData{};
  std::array<Image, 6> mTmpFaceData{};

  void UploadToGpu();

public:
  Type static constexpr SerializationType{ Type::Cubemap };
  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

  explicit LEOPPHAPI Cubemap(std::span<Image, 6> faces);

  LEOPPHAPI auto Update() noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;


  enum FaceIndex {
    Right = 0,
    Left  = 1,
    Up    = 2,
    Down  = 3,
    Front = 4,
    Back  = 5
  };
};
}
