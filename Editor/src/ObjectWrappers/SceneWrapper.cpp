#include <imgui.h>

#include "ObjectWrappers.hpp"
#include "../EditorContext.hpp"
#include "../AssetLoaders/SceneLoader.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<Scene>::OnDrawProperties(Context& context, Object& object) -> void {
  ImGui::Text("%s", "Scene Asset");
  if (ImGui::Button("Open")) {
    context.OpenScene(dynamic_cast<Scene&>(object));
  }
}


auto ObjectWrapperFor<Scene>::GetLoader() -> AssetLoader& {
  SceneLoader static sceneImporter;
  return sceneImporter;
}
}
