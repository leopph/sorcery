#include "SkyboxComponent.hpp"
#include "Renderer.hpp"

#include <imgui.h>
#include <imgui_stdlib.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::SkyboxComponent>{ "SkyboxComponent" }
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
  static std::vector<Cubemap*> cubemaps;
  static std::string cubemapFilter;

  auto constexpr cubemapSelectPopupId{ "Select Cubemap" };
  auto constexpr cubemapNullName{ "None" };

  if (ImGui::Button("Select##Cubemap")) {
    Object::FindObjectsOfType(cubemaps);
    cubemaps.insert(std::begin(cubemaps), nullptr);
    cubemapFilter.clear();
    ImGui::OpenPopup(cubemapSelectPopupId);
  }

  if (ImGui::BeginPopup(cubemapSelectPopupId)) {
    if (ImGui::InputText("##Filter", &cubemapFilter)) {
      Object::FindObjectsOfType(cubemaps);
      std::erase_if(cubemaps, [](Cubemap const* cubemap) {
        return cubemap && !Contains(cubemap->GetName(), cubemapFilter);
      });
    }

    for (auto const cubemap : cubemaps) {
      ImGui::PushID(cubemap
                      ? cubemap->GetGuid().ToString().c_str()
                      : "Cubemap None Id");
      if (ImGui::Selectable(cubemap
                              ? cubemap->GetName().data()
                              : cubemapNullName)) {
        SetCubemap(cubemap);
      }
      ImGui::PopID();
    }

    ImGui::EndPopup();
  }

  ImGui::SameLine();
  ImGui::Text("%s", GetCubemap()
                      ? GetCubemap()->GetName().data()
                      : cubemapNullName);
}
}
