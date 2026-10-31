#pragma once

#include <memory>
#include <vector>

#include <sorcery/observer_ptr.hpp>
#include <sorcery/viewport.hpp>
#include <sorcery/host/rendering/view_history_key.hpp>
#include <sorcery/rendering/render_target.hpp>


namespace sorcery::rendering {
class Camera;


struct View {
  ObserverPtr<Camera const> camera;
  std::shared_ptr<RenderTarget> target;
  NormalizedViewport viewport;
  ViewHistoryKey history;
};


using ViewList = std::vector<View>;
}
