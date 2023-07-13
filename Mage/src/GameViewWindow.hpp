#pragma once

#include "RenderTarget.hpp"

#include <memory>


namespace sorcery::mage {
class GameViewWindow {
  std::unique_ptr<RenderTarget> mRenderTarget;

public:
  auto Draw(bool gameRunning) -> void;
};
}
