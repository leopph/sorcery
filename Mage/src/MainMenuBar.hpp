#pragma once

#include "Application.hpp"
#include "SettingsWindow.hpp"


namespace sorcery::mage {
class MainMenuBar {
  Application* mApp;
  SettingsWindow* mEditorSettingsWindow;

public:
  MainMenuBar(Application& app, SettingsWindow& editorSettingsWindow);

  auto Draw() -> void;
};
}
