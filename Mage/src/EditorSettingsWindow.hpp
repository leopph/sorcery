#pragma once

#include "Application.hpp"

#include <string_view>


namespace sorcery::mage {
class EditorSettingsWindow {
  bool mIsOpen;
  Application* mApp;

public:
  static std::string_view constexpr TITLE{"Editor Settings"};
  static auto constexpr DEFAULT_TARGET_FRAME_RATE{200};

  explicit EditorSettingsWindow(Application& app);
  auto Draw() -> void;
  auto SetOpen(bool isOpen) noexcept -> void;
};
}
