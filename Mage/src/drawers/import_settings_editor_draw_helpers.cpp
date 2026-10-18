#include "import_settings_editor_draw_helpers.hpp"

#include <array>

#include <imgui.h>

#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto DrawMaterialImportSettings(
  [[maybe_unused]] MaterialImportSettings& settings,
  [[maybe_unused]] bool const allow_edit,
  [[maybe_unused]] bool& changed
) -> bool {
  return false;
}


auto DrawTextureImportSettings(
  TextureImportSettings& settings,
  bool const allow_edit,
  bool& changed
) -> bool {
  auto this_func_changed{false};

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
      this_func_changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Color Texture (sRGB)");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##sRGBTex", &settings.is_srgb);
    })) {
      this_func_changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Allow block compression");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##blockCompressCheckbox", &settings.allow_block_compression);
    })) {
      this_func_changed = true;
    }
    ImGui::TableNextColumn();

    ImGui::Text("Generate mipmaps");
    ImGui::TableNextColumn();

    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::Checkbox("##generateMips", &settings.generate_mips);
    })) {
      this_func_changed = true;
    }

    ImGui::TableNextColumn();

    ImGui::Text("Alpha coverage");
    ImGui::TableNextColumn();

    std::array constexpr coverage_mode_strs{"Auto", "Disabled", "Custom"};
    auto coverage_mode_idx{static_cast<int>(settings.alpha_coverage_mode)};

    if (ImGuiDisabled(!allow_edit || !settings.generate_mips, [&] {
      return ImGui::Combo("##alphaCoverageModeCombo", &coverage_mode_idx, coverage_mode_strs.data(),
        clamp_cast<int>(coverage_mode_strs.size()));
    })) {
      settings.alpha_coverage_mode = static_cast<AlphaCoverageMode>(coverage_mode_idx);
      this_func_changed = true;
    }

    if (settings.alpha_coverage_mode == AlphaCoverageMode::kCustom) {
      ImGui::TableNextColumn();

      ImGui::Text("Alpha coverage threshold");
      ImGui::TableNextColumn();

      if (ImGuiDisabled(!allow_edit || !settings.generate_mips, [&] {
        return ImGui::SliderFloat("##alphaCoverageThreshold", &settings.alpha_coverage_threshold, 0.01f, 0.99f);
      })) {
        this_func_changed = true;
      }
    }

    ImGui::EndTable();
  }

  changed = changed || this_func_changed;
  return this_func_changed;
}
}
