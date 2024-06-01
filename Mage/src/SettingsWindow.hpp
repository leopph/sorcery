#pragma once

#include "Cubemap.hpp"
#include "GUI.hpp"
#include "StandaloneCamera.hpp"

#include <string_view>


namespace sorcery::mage {
class EditorApp;


class SettingsWindow {
  EditorApp* mApp;
  StandaloneCamera* mSceneViewCam;
  ObjectPicker<Cubemap> mSkyboxPicker;

public:
  static std::string_view constexpr TITLE{"Settings"};
  static auto constexpr DEFAULT_TARGET_FRAME_RATE{200};

  explicit SettingsWindow(EditorApp& app, StandaloneCamera& sceneViewCam);
  auto Draw() -> void;
};
}
