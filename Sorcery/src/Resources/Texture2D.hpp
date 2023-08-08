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
  Microsoft::WRL::ComPtr<ID3D11Texture2D> mTex{nullptr};
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mSrv{nullptr};
  std::unique_ptr<Image> mImgData{nullptr};

  int mWidth{0};
  int mHeight{0};
  int mChannelCount{0};

  auto UploadToGpu(bool allowBlockCompression) -> void;

public:
  Texture2D() = default;
  LEOPPHAPI explicit Texture2D(Image img, bool keepDataInCpuMemory = false, bool allowBlockCompression = true);

  [[nodiscard]] LEOPPHAPI auto GetImageData() const noexcept -> ObserverPtr<Image const>;
  LEOPPHAPI auto SetImageData(Image img) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView>;

  LEOPPHAPI auto Update(bool keepDataInCpuMemory = false, bool allowBlockCompression = true) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto HasCpuMemory() const noexcept -> bool;
  LEOPPHAPI auto ReleaseCpuMemory() -> void;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> int;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
