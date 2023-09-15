#pragma once

#include "Application.hpp"
#include "StandaloneCamera.hpp"
#include "RenderTarget.hpp"


namespace sorcery::mage {
class SceneViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;
  StandaloneCamera mCam{Vector3{}, Quaternion{}, 5.0f, 0.1f, 1000.0f, 60};

public:
  auto Draw(Application& context) -> void;
  [[nodiscard]] auto GetCamera() noexcept -> StandaloneCamera&;
};
}
