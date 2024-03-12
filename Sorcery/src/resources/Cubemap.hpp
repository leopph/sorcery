#pragma once

#include "Resource.hpp"
#include "../Image.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)

  graphics::SharedDeviceChildHandle<graphics::Texture> tex_;

public:
  LEOPPHAPI Cubemap(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;
};
}
