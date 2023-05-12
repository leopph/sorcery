#include <imgui.h>

#include "ObjectWrappers.hpp"
#include "../EditorContext.hpp"
#include "../SceneImporter.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Scene>::OnDrawProperties(Context& context, Object& object) -> void {
  ImGui::Text("%s", "Scene Asset");
  if (ImGui::Button("Open")) {
    context.OpenScene(dynamic_cast<Scene&>(object));
  }
}


auto ObjectWrapperFor<Scene>::GetImporter() -> Importer& {
  SceneImporter static sceneImporter;
  return sceneImporter;
}
}
