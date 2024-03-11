#pragma once

#include "graphics.hpp"
#include "Camera.hpp"
#include "shadow_atlas.hpp"
#include "../scene_objects/LightComponents.hpp"

#include <array>
#include <span>


namespace sorcery {
class PunctualShadowAtlas final : public ShadowAtlas {
  std::array<Cell, 4> cells_;

public:
  PunctualShadowAtlas(graphics::GraphicsDevice* device, DXGI_FORMAT depth_format, UINT size);

  auto Update(std::span<LightComponent const* const> all_lights, std::span<int const> visible_light_indices,
              Camera const& cam, Matrix4 const& cam_view_proj_mtx, float shadow_distance) -> void;

  [[nodiscard]] auto GetCell(int idx) const -> Cell const& override;
};
}
