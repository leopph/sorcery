#include "Widgets.hpp"

#include <imgui_internal.h>

#include <cmath>


namespace ImGui {
auto Spinner(char const* label, float const radius, int const thickness, ImU32 const& color) -> bool {
  ImGuiWindow* window = GetCurrentWindow();
  if (window->SkipItems) {
    return false;
  }

  auto const& g{ *GImGui };
  ImGuiStyle const& style = g.Style;
  ImGuiID const id{ window->GetID(label) };

  auto const pos{ window->DC.CursorPos };
  ImVec2 const size{ (radius) * 2, (radius + style.FramePadding.y) * 2 };

  ImRect const bb{ pos, ImVec2(pos.x + size.x, pos.y + size.y) };
  ItemSize(bb, style.FramePadding.y);
  if (!ItemAdd(bb, id)) {
    return false;
  }

  // Render
  window->DrawList->PathClear();

  int constexpr num_segments{ 30 };
  float const start{ std::abs(ImSin(static_cast<float>(g.Time) * 1.8f) * (num_segments - 5)) };

  float const a_min = IM_PI * 2.0f * static_cast<float>(start) / static_cast<float>(num_segments);
  float constexpr a_max = IM_PI * 2.0f * (static_cast<float>(num_segments) - 3) / static_cast<float>(num_segments);

  auto const centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

  for (int i = 0; i < num_segments; i++) {
    float const a = a_min + static_cast<float>(i) / static_cast<float>(num_segments) * (a_max - a_min);
    window->DrawList->PathLineTo(ImVec2{ centre.x + ImCos(a + static_cast<float>(g.Time) * 8) * radius, centre.y + ImSin(a + static_cast<float>(g.Time) * 8) * radius });
  }

  window->DrawList->PathStroke(color, false, static_cast<float>(thickness));
  return true;
}
}


namespace sorcery::mage {
auto DrawSpinner(char const* const label, float const radius, int const thickness, ImU32 const& color) -> bool {
  return ImGui::Spinner(label, radius, thickness, color);
}
}
