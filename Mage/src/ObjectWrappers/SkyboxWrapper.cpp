#include <imgui.h>
#include <imgui_stdlib.h>

#include "ObjectWrappers.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<SkyboxComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& skybox{ dynamic_cast<SkyboxComponent&>(object) };

  ImGui::PushID(skybox.GetGuid().ToString().c_str());

  if (ImGui::BeginTable("Table", 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
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
        ImGui::PushID(cubemap ? cubemap->GetGuid().ToString().c_str() : "Cubemap None Id");
        if (ImGui::Selectable(cubemap ? cubemap->GetName().data() : cubemapNullName)) {
          skybox.SetCubemap(cubemap);
        }
        ImGui::PopID();
      }

      ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::Text("%s", skybox.GetCubemap() ? skybox.GetCubemap()->GetName().data() : cubemapNullName);

    ImGui::EndTable();
  }

  ImGui::PopID();
}
}
