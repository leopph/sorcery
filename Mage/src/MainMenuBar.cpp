#include "MainMenuBar.hpp"

#include "ProjectSettingsWindow.hpp"
#include "EditorSettingsWindow.hpp"

#include <nfd.h>


namespace sorcery::mage {
MainMenuBar::MainMenuBar(Application& app, EditorSettingsWindow& editorSettingsWindow) :
  mApp{ &app },
  mEditorSettingsWindow{ &editorSettingsWindow } { }


auto MainMenuBar::Draw() -> void {
  auto static showDemoWindow{ false };
  auto static showProjectSettingsWindow{ false };

  if (showDemoWindow) {
    ImGui::ShowDemoWindow();
  }

  if (showProjectSettingsWindow) {
    DrawProjectSettingsWindow(showProjectSettingsWindow);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open Project")) {
        if (nfdchar_t* selectedPath{ nullptr }; NFD_PickFolder(nullptr, &selectedPath) == NFD_OKAY) {
          mApp->ExecuteInBusyEditor([selectedPath, this] {
            mApp->OpenProject(selectedPath);
            std::free(selectedPath);
          });
        }
      }

      if (ImGui::MenuItem("Save Current Scene")) {
        if (mApp->GetScene()) {
          mApp->GetScene()->Save();
          mApp->GetResourceDatabase().SaveResource(*mApp->GetScene());
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
