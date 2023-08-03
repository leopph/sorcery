#include "MainMenuBar.hpp"

#include "ProjectSettingsWindow.hpp"
#include "EditorSettingsWindow.hpp"
#include "NativeResourceImporter.hpp"
#include "ResourceManager.hpp"

#include <nfd.h>


namespace sorcery::mage {
MainMenuBar::MainMenuBar(Application& app, EditorSettingsWindow& editorSettingsWindow) :
  mApp{&app},
  mEditorSettingsWindow{&editorSettingsWindow} { }


auto MainMenuBar::Draw() -> void {
  auto static showDemoWindow{false};
  auto static showProjectSettingsWindow{false};

  if (showDemoWindow) {
    ImGui::ShowDemoWindow();
  }

  if (showProjectSettingsWindow) {
    DrawProjectSettingsWindow(showProjectSettingsWindow);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene")) {
        mApp->OpenScene(*new Scene{});
      }

      if (ImGui::MenuItem("Open Scene")) {
        if (nfdchar_t* dstPathAbs{nullptr}; NFD_OpenDialog(NativeResourceImporter::SCENE_FILE_EXT.substr(1).data(), mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath().string().c_str(), &dstPathAbs) == NFD_OKAY) {
          if (auto const dstPathResDirRel{relative(dstPathAbs, mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())}; !dstPathResDirRel.empty()) {
            auto const scene{gResourceManager.Load<Scene>(mApp->GetResourceDatabase().PathToGuid(dstPathResDirRel))};
            assert(scene);
            mApp->OpenScene(*scene);

            std::free(dstPathAbs);
          }
        }
      }

      if (ImGui::MenuItem("Save Scene")) {
        mApp->SaveCurrentSceneToFile();
      }

      ImGui::Separator();

      if (ImGui::MenuItem("Open Project")) {
        if (nfdchar_t* selectedPath{nullptr}; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
          mApp->OpenProject(selectedPath);
          std::free(selectedPath);
        }
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Options")) {
      if (ImGui::MenuItem(EditorSettingsWindow::TITLE.data())) {
        mEditorSettingsWindow->SetOpen(true);
      }

      if (ImGui::MenuItem(PROJECT_SETTINGS_WINDOW_TITLE.data())) {
        showProjectSettingsWindow = true;
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
      if (showDemoWindow) {
        if (ImGui::MenuItem("Hide Demo Window")) {
          showDemoWindow = false;
        }
      } else {
        if (ImGui::MenuItem("Show Demo Window")) {
          showDemoWindow = true;
        }
      }

      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}
}
