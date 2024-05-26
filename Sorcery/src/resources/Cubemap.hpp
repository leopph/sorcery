#pragma once

#include "Resource.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)
  graphics::SharedDeviceChildHandle<graphics::Texture> tex_;

public:
  LEOPPHAPI explicit Cubemap(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept;
  Cubemap(Cubemap const&) = delete;
  Cubemap(Cubemap&&) noexcept = delete;

  LEOPPHAPI ~Cubemap() override;

  auto operator=(Cubemap const&) -> void = delete;
  auto operator=(Cubemap&&) noexcept -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;
};
}
