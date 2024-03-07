#pragma once

#include "graphics.hpp"
#include "Camera.hpp"
#include "ShadowAtlas.hpp"
#include "Visibility.hpp"
#include "../scene_objects/LightComponents.hpp"

#include <array>


namespace sorcery {
class PunctualShadowAtlas final : public ShadowAtlas {
  std::array<Cell, 4> cells_;

public:
  PunctualShadowAtlas(graphics::GraphicsDevice* device, UINT size);

  auto Update(std::span<LightComponent const* const> all_lights, Visibility const& visibility, Camera const& cam,
              Matrix4 const& cam_view_proj_mtx, float shadow_distance) -> void;

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
};
}
