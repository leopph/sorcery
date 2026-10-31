#include "sorcery/host/game_views.hpp"

#include <algorithm>
#include <functional>
#include <vector>

#include "sorcery/world.hpp"
#include "sorcery/scene/CameraComponent.hpp"


namespace sorcery {
auto CollectGameViews(World const& world, std::shared_ptr<rendering::RenderTarget> const& default_rt,
                      rendering::ViewList& out) -> void {
  std::vector<ObserverPtr<CameraComponent const>> cameras{std::from_range, world.GetCameras()};

  // Stable so cameras of equal order keep registration order.
  std::ranges::stable_sort(cameras, std::less{}, &CameraComponent::GetOrder);

  for (auto const cam : cameras) {
    auto const& cam_rt = cam->GetRenderTarget();

    if (!cam_rt && !default_rt) {
      continue;
    }

    out.emplace_back(cam, cam_rt ? cam_rt : default_rt, cam->GetViewport(),
      rendering::ViewHistoryKey::ForObject(cam->GetId()));
  }
}
}
