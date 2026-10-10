#pragma once

#include "observer_ptr.hpp"
#include "rendering/render_target.hpp"


namespace sorcery::mage {
class ImGuiTextureReferences;


class GameViewWindow {
public:
  explicit GameViewWindow(ImGuiTextureReferences& tex_refs);

  auto Draw(bool game_is_running) -> void;

private:
  std::shared_ptr<rendering::RenderTarget> rt_override_;
  ObserverPtr<ImGuiTextureReferences> tex_refs_;
  int resolution_mode_idx_{0};
  bool was_visible_{false};
};
}
