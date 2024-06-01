#pragma once

#include "SettingsWindow.hpp"


namespace sorcery::mage {
class EditorApp;


class MainMenuBar {
  EditorApp* mApp;
  SettingsWindow* mEditorSettingsWindow;

public:
  MainMenuBar(EditorApp& app, SettingsWindow& editorSettingsWindow);

  auto Draw() -> void;
};
}
