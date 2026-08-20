#include "editor_drawer_registry.hpp"

#include <utility>

#include "camera_component_editor_drawer.hpp"
#include "camera_controller_component_editor_drawer.hpp"
#include "component_editor_drawer.hpp"
#include "entity_editor_drawer.hpp"
#include "light_component_editor_drawer.hpp"
#include "material_editor_drawer.hpp"
#include "mesh_component_base_editor_drawer.hpp"
#include "mesh_editor_drawer.hpp"
#include "model_importer_editor_drawer.hpp"
#include "object_editor_drawer.hpp"
#include "oscillate_component_editor_drawer.hpp"
#include "skinned_mesh_component_editor_drawer.hpp"
#include "texture2d_editor_drawer.hpp"
#include "texture_importer_editor_drawer.hpp"
#include "transform_component_editor_drawer.hpp"


namespace sorcery::mage {
auto EditorDrawerRegistry::RegisterDrawer(std::unique_ptr<EditorDrawerBase> drawer) -> void {
  drawers_[drawer->GetTargetType()] = std::move(drawer);
}


EditorDrawerRegistry::EditorDrawerRegistry() {
  RegisterDrawer(std::make_unique<ObjectEditorDrawer>());
  RegisterDrawer(std::make_unique<MaterialEditorDrawer>());
  RegisterDrawer(std::make_unique<MeshEditorDrawer>());
  RegisterDrawer(std::make_unique<Texture2DEditorDrawer>());
  RegisterDrawer(std::make_unique<CameraComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<CameraControllerComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<ComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<EntityEditorDrawer>());
  RegisterDrawer(std::make_unique<LightComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<MeshComponentBaseEditorDrawer>());
  RegisterDrawer(std::make_unique<OscillateComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<SkinnedMeshComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<TransformComponentEditorDrawer>());
  RegisterDrawer(std::make_unique<TextureImporterEditorDrawer>());
  RegisterDrawer(std::make_unique<ModelImporterEditorDrawer>());
}
}
