#include "LoadingScreen.hpp"

#include <imgui.h>

#include "Widgets.hpp"


namespace sorcery::mage {
auto DrawLoadingScreen(Context& context) -> void {
  ImGui::SetNextWindowPos(ImVec2(context.GetImGuiIo().DisplaySize.x * 0.5f, context.GetImGuiIo().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  if (ImGui::Begin("LoadingIndicator", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse)) {
    DrawSpinner("##spinner", 15, 6, ImGui::GetColorU32(ImGuiCol_ButtonHovered));
  }

  ImGui::End();
}
}
