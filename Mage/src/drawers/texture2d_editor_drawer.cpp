#include "texture2d_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"


namespace sorcery::mage {
auto Texture2DEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  Texture2D& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Resource>(obj, allow_edit, changed);

  if (ImGui::BeginTable(std::format("{}", obj.GetResId().GetGuid().ToString()).c_str(), 2,
    ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Width");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(obj.GetWidth()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Height");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(obj.GetHeight()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Channel Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(obj.GetChannelCount()).c_str());

    ImGui::EndTable();
  }

  auto const contentRegion{ImGui::GetContentRegionAvail()};
  auto const imgWidth{static_cast<float>(obj.GetWidth())};
  auto const imgHeight{static_cast<float>(obj.GetHeight())};
  auto const widthRatio{contentRegion.x / imgWidth};
  auto const heightRatio{contentRegion.y / imgHeight};
  ImVec2 displaySize;

  if (widthRatio > heightRatio) {
    displaySize.x = imgWidth * heightRatio;
    displaySize.y = imgHeight * heightRatio;
  } else {
    displaySize.x = imgWidth * widthRatio;
    displaySize.y = imgHeight * widthRatio;
  }

  ImGui::Image(std::bit_cast<ImTextureID>(obj.GetTex().get()), displaySize);
}
}
