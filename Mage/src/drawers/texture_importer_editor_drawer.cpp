#include "texture_importer_editor_drawer.hpp"

#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto TextureImporterEditorDrawer::Draw(
  [[maybe_unused]] EditorDrawerContext const& ctx,
  TextureImporter& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  auto settings{obj.GetSettings()};

  if (ImGui::BeginTable("##texImporterTable", 2)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text("Texture Type");
    ImGui::TableNextColumn();

    std::array constexpr tex_type_strs{"Texture 2D", "Cubemap"};
    auto tex_type_idx{static_cast<int>(settings.type)};

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Combo("##texTypeCombo", &tex_type_idx, tex_type_strs.data(), clamp_cast<int>(tex_type_strs.size()));
    })) {
      settings.type = static_cast<TextureImportType>(tex_type_idx);
      obj.SetSettings(settings);
      changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Color Texture (sRGB)");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##sRGBTex", &settings.is_srgb);
    })) {
      obj.SetSettings(settings);
      changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Allow block compression");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##blockCompressCheckbox", &settings.allow_block_compression);
    })) {
      obj.SetSettings(settings);
      changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Generate mipmaps");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##generateMips", &settings.generate_mips);
    })) {
      obj.SetSettings(settings);
      changed = true;
    }

    ImGui::EndTable();
  }
}
}
