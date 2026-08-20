#pragma once

#include <imgui.h>
#include <ImGuizmo.h>
#include <implot.h>


namespace sorcery::mage {
auto DrawSpinner(char const* label, float radius, int thickness, ImU32 const& color) -> bool;

template<typename T>
auto ReflectionDisplayProperties(T& obj) -> void;
}


#include "editor_gui_helpers.inl"
