#pragma once

#include "sorcery/observer_ptr.hpp"
#include "windows/SettingsWindow.hpp"


namespace sorcery {
class GameModule;


namespace mage {
class EditorApp;


class MainMenuBar {
public:
  MainMenuBar(EditorApp& app, SettingsWindow& editor_settings_window, GameModule& game_module);

  auto Draw() -> void;

private:
  ObserverPtr<EditorApp> app_;
  ObserverPtr<SettingsWindow> editor_settings_window_;
  ObserverPtr<GameModule> game_module_;
};
}
}
