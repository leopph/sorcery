#include "StartupScreen.hpp"

#include <nfd.h>


namespace sorcery::mage {
auto DrawStartupScreen(EditorApp& context) -> void {
  auto constexpr flags{ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings};
  auto const viewport{ImGui::GetMainViewport()};
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);

  if (ImGui::Begin("Open Project##OpenProjectWindow", nullptr, flags)) {
    auto const openProjectButtonLabel{"Open Project##OpenProjectButton"};
    auto const windowSize{ImGui::GetWindowSize()};
    auto const textSize{ImGui::CalcTextSize(openProjectButtonLabel)};

    ImGui::SetCursorPosX((windowSize.x - textSize.x) * 0.5f);
    ImGui::SetCursorPosY((windowSize.y - textSize.y) * 0.5f);

    if (ImGui::Button(openProjectButtonLabel)) {
      if (nfdchar_t* selectedPath{nullptr}; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
        context.OpenProject(selectedPath);
        std::free(selectedPath);
      }
    }
  }
  ImGui::End();
}
}
