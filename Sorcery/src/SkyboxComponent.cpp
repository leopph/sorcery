#include "SkyboxComponent.hpp"
#include "Renderer.hpp"
#include "GUI.hpp"

#include <imgui.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SkyboxComponent>{"SkyboxComponent"}
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("cubemap", &sorcery::SkyboxComponent::GetCubemap, &sorcery::SkyboxComponent::SetCubemap);
}


namespace sorcery {
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


auto SkyboxComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("%s", "Cubemap");

  ImGui::TableNextColumn();

  static ObjectPicker<Cubemap> cubemapPicker;
  if (auto cubemap{GetCubemap()}; cubemapPicker.Draw(cubemap, true)) {
    SetCubemap(cubemap);
  }
}
}
