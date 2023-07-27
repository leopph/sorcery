#include <imgui.h>

#include "ObjectWrappers.hpp"
#include "../EditorContext.hpp"


namespace sorcery::mage {
auto ObjectWrapperFor<Scene>::OnDrawProperties(Context& context, Object& object) -> void {
  ImGui::Text("%s", "Scene Asset");
  if (ImGui::Button("Open")) {
    context.OpenScene(dynamic_cast<Scene&>(object));
  }
}
}
