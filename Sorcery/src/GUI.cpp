#include "GUI.hpp"

#include <memory>


namespace sorcery {
auto SetImGuiContext(ImGuiContext& ctx) -> void {
  ImGui::SetCurrentContext(std::addressof(ctx));
}
}
