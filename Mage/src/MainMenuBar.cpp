#include "MainMenuBar.hpp"

#include "EditorApp.hpp"
#include "SettingsWindow.hpp"

#include <nfd.hpp>


namespace sorcery::mage {
MainMenuBar::MainMenuBar(EditorApp& app, SettingsWindow& editor_settings_window) :
  app_{&app},
  editor_settings_window_{&editor_settings_window} {}


auto MainMenuBar::Draw() -> void {
  auto static showDemoWindow{false};

  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene")) {
        app_->OpenNewScene();
      }

      if (ImGui::MenuItem("Open Scene")) {
        constexpr nfdu8filteritem_t filter_item{"Scene", ResourceManager::SCENE_RESOURCE_EXT.substr(1).data()};

        if (NFD::UniquePath dst_path_abs{nullptr};
          OpenDialog(dst_path_abs, &filter_item, 1,
            app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath().string().c_str()) == NFD_OKAY) {
          if (auto const dst_path_res_dir_rel{
            relative(dst_path_abs.get(), app_->GetResourceDatabase().GetResourceDirectoryAbsolutePath())
          }; !dst_path_res_dir_rel.empty()) {
            app_->OpenScene(app_->GetResourceDatabase().PathToGuid(dst_path_res_dir_rel));
          }
        }
      }

      if (ImGui::MenuItem("Save Scene")) {
        app_->SaveCurrentSceneToFile();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Open Project")) {
        if (NFD::UniquePath selected_path{nullptr}; PickFolder(selected_path, nullptr) == NFD_OKAY) {
          app_->OpenProject(selected_path.get());
        }
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::MenuItem("ImGui Demo Window")) {
        showDemoWindow = true;
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}
}
