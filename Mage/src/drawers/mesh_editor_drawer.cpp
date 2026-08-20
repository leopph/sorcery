#include "mesh_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"


namespace sorcery::mage {
auto MeshEditorDrawer::Draw(EditorDrawerContext const& ctx, Mesh& mesh, bool const allow_edit, bool& changed) -> void {
  ctx.registry->DrawAs<Resource>(mesh, allow_edit, changed);

  ImGui::Text("%s: %d", "Vertex Count", mesh.GetVertexCount());
  ImGui::Text("%s: %d", "Triangle Count", mesh.GetPrimitiveCount());
  ImGui::Text("%s: %d", "Meshlet Count", mesh.GetMeshletCount());
  ImGui::Text("%s: %d", "Submesh Count", mesh.GetSubmeshes().size());
  ImGui::Text("%s: %s", "Uses 32-bit indices", mesh.Has32BitVertexIndices() ? "yes" : "no");
}
}
