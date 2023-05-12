#include "ObjectPropertiesWindow.hpp"


namespace leopph::editor {
auto DrawObjectPropertiesWindow(Context& context) -> void {
  ImGui::SetNextWindowSize(ImVec2{ 400, 600 }, ImGuiCond_FirstUseEver);

  if (ImGui::Begin("Object Properties", nullptr, ImGuiWindowFlags_NoCollapse)) {
    if (context.GetSelectedObject()) {
      context.GetFactoryManager().GetFor(context.GetSelectedObject()->GetSerializationType()).OnDrawProperties(context, *context.GetSelectedObject());
    }
  }
  ImGui::End();
}
}
