#include "MainMenuBar.hpp"

#include "EditorApp.hpp"
#include "SettingsWindow.hpp"

#include <nfd.h>


namespace sorcery::mage {
MainMenuBar::MainMenuBar(EditorApp& app, SettingsWindow& editorSettingsWindow) :
  mApp{&app},
  mEditorSettingsWindow{&editorSettingsWindow} {}


auto MainMenuBar::Draw() -> void {
  auto static showDemoWindow{false};

  if (showDemoWindow) {
    ImGui::ShowDemoWindow(&showDemoWindow);
  }

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New Scene")) {
        mApp->OpenNewScene();
      }

      if (ImGui::MenuItem("Open Scene")) {
        if (nfdchar_t* dstPathAbs{nullptr}; NFD_OpenDialog(ResourceManager::SCENE_RESOURCE_EXT.substr(1).data(),
                                              mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath().string().
                                                    c_str(), &dstPathAbs) == NFD_OKAY) {
          if (auto const dstPathResDirRel{
            relative(dstPathAbs, mApp->GetResourceDatabase().GetResourceDirectoryAbsolutePath())
          }; !dstPathResDirRel.empty()) {
            mApp->OpenScene(mApp->GetResourceDatabase().PathToGuid(dstPathResDirRel));

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
