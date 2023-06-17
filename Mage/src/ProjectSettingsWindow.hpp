#pragma once

#include <string_view>


namespace sorcery::mage {
extern std::string_view const PROJECT_SETTINGS_WINDOW_TITLE;

auto DrawProjectSettingsWindow(bool& isOpen) -> void;
}
