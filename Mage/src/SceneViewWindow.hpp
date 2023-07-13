#pragma once

#include "EditorContext.hpp"
#include "RenderTarget.hpp"


namespace sorcery::mage {
class SceneViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;

public:
  auto Draw(Context& context) -> void;
};
}
