#pragma once

#include "rendering/render_target.hpp"

namespace sorcery::mage {
class GameViewWindow {
public:
  auto Draw(bool game_is_running) -> void;

private:
  std::shared_ptr<rendering::RenderTarget> rt_override_;
  int resolution_mode_idx_{0};
};
}
