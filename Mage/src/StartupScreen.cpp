#include "StartupScreen.hpp"

#include <nfd.hpp>


namespace sorcery::mage {
auto DrawStartupScreen(EditorApp& context) -> void {
  auto constexpr flags{ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings};
  auto const viewport{ImGui::GetMainViewport()};
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);

  if (ImGui::Begin("Open Project##OpenProjectWindow", nullptr, flags)) {
    auto const open_project_button_label{"Open Project##OpenProjectButton"};
    auto const window_size{ImGui::GetWindowSize()};
    auto const text_size{ImGui::CalcTextSize(open_project_button_label)};

    ImGui::SetCursorPosX((window_size.x - text_size.x) * 0.5f);
    ImGui::SetCursorPosY((window_size.y - text_size.y) * 0.5f);

    if (ImGui::Button(open_project_button_label)) {
      if (NFD::UniquePathN selected_path; PickFolder(selected_path, nullptr) == NFD_OKAY) {
        context.OpenProject(selected_path.get());
      }
    }
  }
  ImGui::End();
}
}
