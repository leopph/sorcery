#pragma once

#include "RenderTarget.hpp"
#include "EditorCamera.hpp"

#include <memory>


namespace sorcery::mage {
class GameViewWindow {
  std::unique_ptr<RenderTarget> mHdrRenderTarget;
  std::unique_ptr<RenderTarget> mFinalRenderTarget;

public:
  auto Draw(bool gameRunning, EditorCamera const& editorCam) -> void;
};
}
