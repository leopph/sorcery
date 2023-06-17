#pragma once

#include <string_view>


namespace sorcery::mage {
extern std::string_view const EDITOR_SETTINGS_WINDOW_TITLE;
auto constexpr DEFAULT_TARGET_FRAME_RATE{ 200 };

auto DrawEditorSettingsWindow(bool& isOpen) -> void;
}
