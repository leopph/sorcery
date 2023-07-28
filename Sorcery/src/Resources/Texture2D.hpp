#pragma once

#include "Resource.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>

#include "../Image.hpp"
#include "../Util.hpp"


namespace sorcery {
class Texture2D final : public Resource {
  RTTR_ENABLE(Resource)
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv;
  Image* mImgData{ new Image{} };

  int mWidth{ 0 };
  int mHeight{ 0 };
  int mChannelCount{ 0 };

  auto UploadToGPU() -> void;

public:
  Texture2D() = default;
  LEOPPHAPI explicit Texture2D(Image img, bool keepDataInCPUMemory = false);

  [[nodiscard]] LEOPPHAPI auto GetImageData() const noexcept -> ObserverPtr<Image const>;
  LEOPPHAPI auto SetImageData(Image img, bool allocateCPUMemoryIfNeeded = false) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;

  LEOPPHAPI auto Update(bool keepDataInCPUMemory = false) noexcept -> void;

  LEOPPHAPI auto ReleaseCPUMemory() -> void;
  [[nodiscard]] LEOPPHAPI auto HasCPUMemory() const noexcept -> bool;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> int;

  LEOPPHAPI auto OnDrawProperties() -> void override;
};
}
