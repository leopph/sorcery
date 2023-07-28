#pragma once

#include "Application.hpp"
#include "EditorCamera.hpp"
#include "RenderTarget.hpp"


namespace sorcery::mage {
class SceneViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;
  EditorCamera mEditorCam{ Vector3{}, Quaternion{}, 0.03f, 10000.f, 90 };

public:
  auto Draw(Application& context) -> void;
};
}
