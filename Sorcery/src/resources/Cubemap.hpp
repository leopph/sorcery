#pragma once

#include "Resource.hpp"
#include "wand/device_object.hpp"
#include "wand/texture.hpp"


namespace sorcery {
class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)
  wand::SharedDeviceHandle<wand::Texture> tex_;

public:
  LEOPPHAPI explicit Cubemap(wand::SharedDeviceHandle<wand::Texture> tex) noexcept;
  Cubemap(Cubemap const&) = delete;
  Cubemap(Cubemap&&) noexcept = delete;

  LEOPPHAPI ~Cubemap() override;

  auto operator=(Cubemap const&) -> void = delete;
  auto operator=(Cubemap&&) noexcept -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const&;
};
}
