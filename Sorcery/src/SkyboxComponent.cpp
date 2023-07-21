#include "SkyboxComponent.hpp"

#include "Renderer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SkyboxComponent>{ "SkyboxComponent" }
    .REFLECT_REGISTER_COMPONENT_CTOR;
}


namespace sorcery {
auto SkyboxComponent::GetSerializationType() const -> Type {
  return SerializationType;
}


auto SkyboxComponent::GetCubemap() const noexcept -> Cubemap* {
  return mCubemap;
}


auto SkyboxComponent::SetCubemap(Cubemap* cubemap) noexcept -> void {
  gRenderer.UnregisterSkybox(this);

  mCubemap = cubemap;

  if (mCubemap) {
    gRenderer.RegisterSkybox(this);
  }
}


SkyboxComponent::~SkyboxComponent() {
  gRenderer.UnregisterSkybox(this);
}
}
