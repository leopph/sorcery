#pragma once

#include "Application.hpp"
#include "StandaloneCamera.hpp"
#include "RenderTarget.hpp"

#include <optional>


namespace sorcery::mage {
class SceneViewWindow {
  struct FocusMoveInfo {
    Vector3 source;
    Vector3 target;
    float t;
  };


  std::unique_ptr<RenderTarget> mRenderTarget;
  StandaloneCamera mCam{Vector3{}, Quaternion{}, 5.0f, 0.1f, 1000.0f, 60};
  std::optional<FocusMoveInfo> mFocusTarget;

public:
  auto Draw(Application& context) -> void;
  [[nodiscard]] auto GetCamera() noexcept -> StandaloneCamera&;
};
}
