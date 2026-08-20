#include "mesh_component_base_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto MeshComponentBaseEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  MeshComponentBase& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::Text("%s", "Mesh");
  ImGui::TableNextColumn();
  static ObjectPicker<Mesh> meshPicker;
  if (auto mesh{obj.GetMesh()}; ImGuiDisabled(!allow_edit, [&] {
    return meshPicker.Draw(mesh, true);
  })) {
    obj.SetMesh(mesh);
  }

  if (auto const mesh{obj.GetMesh()}) {
    ImGui::TableNextColumn();
    ImGui::Text("%s", "Materials");

    auto const mtlSlots{mesh->GetMaterialSlots()};
    auto const mtlCount{std::ssize(mtlSlots)};

    static std::vector<ObjectPicker<Material>> mtlPickers;
    if (std::ssize(mtlPickers) < mtlCount) {
      mtlPickers.resize(mtlCount);
    }

    auto const mtls{obj.GetMaterials()};

    for (auto i = 0; i < mtlCount; i++) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", mtlSlots[i].name.c_str());
      ImGui::TableNextColumn();
      if (auto mtl{mtls[i]}; ImGuiDisabled(!allow_edit, [&] {
        return mtlPickers[i].Draw(mtl, true);
      })) {
        obj.SetMaterial(i, mtl);
      }
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Show bounding boxes");
  ImGui::TableNextColumn();
  if (auto show_bboxes{MeshComponentBase::IsShowingBoundingBoxes()};
    ImGui::Checkbox("##showAabbsCheckbox", &show_bboxes)) {
    MeshComponentBase::SetShowBoundingBoxes(show_bboxes);
  }
}
}
