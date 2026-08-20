#include "model_importer_editor_drawer.hpp"

#include <ranges>

#include <imgui.h>

#include "import_settings_editor_draw_helpers.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto ModelImporterEditorDrawer::Draw(
  [[maybe_unused]] EditorDrawerContext const& ctx,
  ModelImporter& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ImGui::Separator();

  if (ImGui::BeginTable("##ModelImporterMeshTable", 2)) {
    auto mesh_settings{obj.GetMeshImportSettings()};

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text("Fuse submeshes");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##FuseSubmeshesCheckbox", &mesh_settings.fuse_submeshes);
    })) {
      obj.SetMeshImportSettings(mesh_settings);
      changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Force 32-bit indices");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##32BitIndicesCheckbox", &mesh_settings.force_idx32);
    })) {
      obj.SetMeshImportSettings(mesh_settings);
      changed = true;
    }

    ImGui::EndTable();
  }

  ImGui::Separator();

  if (ImGui::BeginTable("##ModelImporterMiscTable", 2)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text("Import materials");
    ImGui::TableNextColumn();

    auto import_mtls{obj.IsMaterialImportEnabled()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##ImportMaterialEnabled", &import_mtls);
    })) {
      obj.SetMaterialImportEnabled(import_mtls);
      changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Import textures");
    ImGui::TableNextColumn();

    auto import_texs{obj.IsTextureImportEnabled()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##ImportTexturesEnabled", &import_texs);
    })) {
      obj.SetTextureImportEnabled(import_texs);
      changed = true;
    }

    ImGui::EndTable();
  }

  ImGui::Separator();

  auto subres_settings{
    std::ranges::to<std::vector<ModelImporter::SubresourceImportSettings>>(obj.GetSubresourceImportSettings())
  };

  for (auto idx{0}; auto& settings : subres_settings) {
    ImGui::SeparatorText(std::format("Subresource {} ({})", idx,
      settings.kind == ModelImporter::SubresourceKind::kMaterial ? "Material" : "Texture").c_str());

    ImGui::PushID(idx++);

    switch (settings.kind) {
      case ModelImporter::SubresourceKind::kMaterial: {
        if (DrawMaterialImportSettings(settings.material_settings, allow_edit, changed)) {
          obj.SetSubresourceImportSettings(subres_settings);
        }
        break;
      }

      case ModelImporter::SubresourceKind::kTexture: {
        if (DrawTextureImportSettings(settings.texture_settings, allow_edit, changed)) {
          obj.SetSubresourceImportSettings(subres_settings);
        }
        break;
      }
    }

    ImGui::PopID();
  }
}
}
