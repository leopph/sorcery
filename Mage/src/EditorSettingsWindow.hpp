#pragma once

#include <string_view>


namespace sorcery::mage {
class EditorSettingsWindow {
  bool mIsOpen{ false };

public:
  static std::string_view constexpr TITLE{ "Editor Settings" };
  static auto constexpr DEFAULT_TARGET_FRAME_RATE{ 200 };

  auto Draw() -> void;
  auto SetOpen(bool isOpen) noexcept -> void;
};
}
