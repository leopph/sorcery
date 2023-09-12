#pragma once

#include "Application.hpp"
#include "EditorCamera.hpp"
#include "RenderTarget.hpp"


namespace sorcery::mage {
class SceneViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;
  EditorCamera mEditorCam{Vector3{}, Quaternion{}, 0.3f, 1000.0f, 90};

public:
  auto Draw(Application& context) -> void;
  [[nodiscard]] auto GetCamera() noexcept -> EditorCamera&;
};
}
