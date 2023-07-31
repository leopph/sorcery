#pragma once

#include "Application.hpp"
#include "EditorSettingsWindow.hpp"


namespace sorcery::mage {
class MainMenuBar {
  Application* mApp;
  EditorSettingsWindow* mEditorSettingsWindow;

public:
  MainMenuBar(Application& app, EditorSettingsWindow& editorSettingsWindow);

  auto Draw() -> void;
};
}
