#pragma once

#include <imgui.h>


namespace sorcery::mage {
auto DrawSpinner(char const* label, float radius, int thickness, ImU32 const& color) -> bool;
}
