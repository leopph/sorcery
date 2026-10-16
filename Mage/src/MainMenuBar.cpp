#include "MainMenuBar.hpp"

#include <nfd.hpp>

#include "editor_app.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/host/game_module.hpp"
#include "windows/SettingsWindow.hpp"


namespace sorcery::mage {
MainMenuBar::MainMenuBar(EditorApp& app, SettingsWindow& editor_settings_window, GameModule& game_module) :
  app_{&app},
  editor_settings_window_{&editor_settings_window},
  game_module_{&game_module} {}


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
        constexpr nfdu8filteritem_t filter_item{"Scene", AssetManager::SCENE_ASSET_EXT.substr(1).data()};
        auto& asset_db{app_->GetAssetDatabase()};
        auto const& asset_dir_abs_path{asset_db.GetAssetDirectoryAbsolutePath()};
        auto const default_path_str{ToUntypedStr(asset_dir_abs_path.u8string())};

        if (NFD::UniquePathU8 dst_path_abs{nullptr};
          OpenDialog(dst_path_abs, &filter_item, 1, default_path_str.c_str()) == NFD_OKAY) {
          if (auto const dst_path_asset_dir_rel{relative(ToUtf8Sv(dst_path_abs.get()), asset_dir_abs_path)};
            !dst_path_asset_dir_rel.empty()) {
            app_->OpenScene(AssetId{asset_db.PathToGuid(dst_path_asset_dir_rel), 0});
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

      ImGui::Separator();

      if (ImGui::MenuItem("Load Game Module", nullptr, false, !game_module_->IsLoaded())) {
        constexpr nfdu8filteritem_t filter_item{
          .name = "Dynamic Library",
          .spec = "dll"
        };

        NFD::UniquePathU8 path_abs{nullptr};

        if (OpenDialog(path_abs, &filter_item, 1) == NFD_OKAY) {
          game_module_->Load(ToUtf8Sv(path_abs.get()));
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
