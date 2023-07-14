#pragma once

#include "EditorContext.hpp"
#include "EditorCamera.hpp"
#include "RenderTarget.hpp"


namespace sorcery::mage {
class SceneViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;
  EditorCamera mEditorCam{ Vector3{}, Quaternion{}, 0.03f, 10000.f, 90 };

public:
  auto Draw(Context& context) -> void;
};
}
