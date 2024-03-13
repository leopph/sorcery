#include "Cubemap.hpp"

#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Cubemap>{"Cubemap"};
}


namespace sorcery {
Cubemap::Cubemap(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept :
  tex_{std::move(tex)} {}


auto Cubemap::GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return tex_;
}
}
