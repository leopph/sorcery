#include "MainMenuBar.hpp"

#include <nfd.hpp>

#include "char_encoding_helpers.hpp"
#include "EditorApp.hpp"
#include "SettingsWindow.hpp"


namespace sorcery::mage {
MainMenuBar::MainMenuBar(EditorApp& app, SettingsWindow& editor_settings_window) :
  app_{&app},
  editor_settings_window_{&editor_settings_window} {}


auto MainMenuBar::Draw() -> void {
  auto static show_demo_window{false};

  if (show_demo_window) {
    ImGui::ShowDemoWindow(&show_demo_window);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene")) {
        app_->OpenNewScene();
      }

      if (ImGui::MenuItem("Open Scene")) {
        constexpr nfdu8filteritem_t filter_item{"Scene", ResourceManager::SCENE_RESOURCE_EXT.substr(1).data()};
        auto& res_db{app_->GetResourceDatabase()};
        auto const& res_dir_abs_path{res_db.GetResourceDirectoryAbsolutePath()};
        auto const default_path_str{ToUntypedStr(res_dir_abs_path.u8string())};

        if (NFD::UniquePathU8 dst_path_abs{nullptr};
          OpenDialog(dst_path_abs, &filter_item, 1, default_path_str.c_str()) == NFD_OKAY) {
          if (auto const dst_path_res_dir_rel{relative(ToU8StrView(dst_path_abs.get()), res_dir_abs_path)};
            !dst_path_res_dir_rel.empty()) {
            app_->OpenScene(res_db.PathToGuid(dst_path_res_dir_rel));
          }
        }
      }

      if (ImGui::MenuItem("Save Scene")) {
        app_->SaveCurrentSceneToFile();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Open Project")) {
        auto const& proj_dir_abs{app_->GetProjectDirectoryAbsolute()};
        auto const default_path_abs{
          proj_dir_abs.has_parent_path()
            ? proj_dir_abs.parent_path()
            : proj_dir_abs
        };

        if (NFD::UniquePathN dst_path; PickFolder(dst_path, default_path_abs.c_str()) == NFD_OKAY) {
          app_->OpenProject(dst_path.get());
        }
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::MenuItem("ImGui Demo Window")) {
        show_demo_window = true;
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}
}
