#pragma once

#include "SettingsWindow.hpp"


namespace sorcery::mage {
class EditorApp;


class MainMenuBar {
  EditorApp* app_;
  SettingsWindow* editor_settings_window_;

public:
  MainMenuBar(EditorApp& app, SettingsWindow& editor_settings_window);

  auto Draw() -> void;
};
}
