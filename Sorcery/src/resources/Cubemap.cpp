#include "Cubemap.hpp"

#include "../engine_context.hpp"

#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Cubemap>{"Cubemap"};
}


namespace sorcery {
Cubemap::Cubemap(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept :
  tex_{std::move(tex)} {}


Cubemap::~Cubemap() {
  g_engine_context.render_manager->KeepAliveWhileInUse(tex_);
}


auto Cubemap::GetTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return tex_;
}
}
