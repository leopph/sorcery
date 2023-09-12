#pragma once

#include "Application.hpp"
#include "EditorCamera.hpp"

#include <string_view>


namespace sorcery::mage {
class SettingsWindow {
  bool mIsOpen;
  Application* mApp;
  EditorCamera* mSceneViewCam;

public:
  static std::string_view constexpr TITLE{"Settings"};
  static auto constexpr DEFAULT_TARGET_FRAME_RATE{200};

  explicit SettingsWindow(Application& app, EditorCamera& sceneViewCam);
  auto Draw() -> void;
  auto SetOpen(bool isOpen) noexcept -> void;
};
}
