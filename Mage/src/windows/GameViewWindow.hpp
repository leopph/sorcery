#pragma once

#include "sorcery/observer_ptr.hpp"
#include "sorcery/host/rendering/view.hpp"
#include "sorcery/rendering/render_target.hpp"


namespace sorcery {
class World;
}


namespace sorcery::mage {
class ImGuiTextureReferences;


class GameViewWindow {
public:
  explicit GameViewWindow(ImGuiTextureReferences& tex_refs);

  auto Draw(World const& world, bool game_is_running) -> void;
  auto AddViews(World const& world, rendering::ViewList& views) const -> void;

private:
  std::shared_ptr<rendering::RenderTarget> rt_;
  ObserverPtr<ImGuiTextureReferences> tex_refs_;
  int resolution_mode_idx_{0};
  bool was_visible_{false};
};
}
