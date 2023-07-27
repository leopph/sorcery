#include <imgui.h>

#include "ObjectWrappers.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<Texture2D>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto const& tex{ dynamic_cast<Texture2D&>(object) };

  if (ImGui::BeginTable(std::format("{}", tex.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Width");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(tex.GetWidth()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Height");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(tex.GetHeight()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Channel Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(tex.GetChannelCount()).c_str());

    ImGui::EndTable();
  }

  auto const contentRegion{ ImGui::GetContentRegionAvail() };
  auto const imgWidth{ static_cast<float>(tex.GetWidth()) };
  auto const imgHeight{ static_cast<float>(tex.GetHeight()) };
  auto const widthRatio{ contentRegion.x / imgWidth };
  auto const heightRatio{ contentRegion.y / imgHeight };
  ImVec2 displaySize;

  if (widthRatio > heightRatio) {
    displaySize.x = imgWidth * heightRatio;
    displaySize.y = imgHeight * heightRatio;
  } else {
    displaySize.x = imgWidth * widthRatio;
    displaySize.y = imgHeight * widthRatio;
  }

  ImGui::Image(tex.GetSrv(), displaySize);
}
}
