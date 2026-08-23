#include "Cubemap.hpp"

#include "../app.hpp"
#include "../rendering/render_manager.hpp"

#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Cubemap>{"Cubemap"};
}


namespace sorcery {
Cubemap::Cubemap(wand::SharedDeviceChildHandle<wand::Texture> tex) noexcept :
  tex_{std::move(tex)} {}


Cubemap::~Cubemap() {
  App::Instance().GetRenderManager().KeepAliveWhileInUse(tex_);
}


auto Cubemap::GetTex() const noexcept -> wand::SharedDeviceChildHandle<wand::Texture> const& {
  return tex_;
}
}
