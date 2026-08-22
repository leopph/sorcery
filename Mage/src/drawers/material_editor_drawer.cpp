#include "material_editor_drawer.hpp"

#include <imgui.h>

#include "editor_drawer_registry.hpp"

#include "../gui_helpers.hpp"


namespace sorcery::mage {
auto MaterialEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  Material& mtl,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<NativeResource>(mtl, allow_edit, changed);

  if (ImGui::BeginTable(std::format("{}", mtl.GetResId().GetGuid().ToString()).c_str(), 2,
    ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Albedo Color");
    ImGui::TableNextColumn();

    if (Vector3 albedoColor{mtl.GetAlbedoVector()}; ImGuiDisabled(!allow_edit, [&] {
      return ImGui::ColorEdit3("##matAlbedoColor", albedoColor.GetData());
    })) {
      mtl.SetAlbedoVector(albedoColor, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Metallic");
    ImGui::TableNextColumn();

    if (f32 metallic{mtl.GetMetallic()}; ImGuiDisabled(!allow_edit, [&] {
      return ImGui::SliderFloat("##matMetallic", &metallic, 0.0f, 1.0f);
    })) {
      mtl.SetMetallic(metallic, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Roughness");
    ImGui::TableNextColumn();

    if (f32 roughness{mtl.GetRoughness()}; ImGuiDisabled(!allow_edit, [&] {
      return ImGui::SliderFloat("##matRoughness", &roughness, 0.0f, 1.0f);
    })) {
      mtl.SetRoughness(roughness, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Ambient Occlusion");
    ImGui::TableNextColumn();

    if (f32 ao{mtl.GetAo()}; ImGuiDisabled(!allow_edit, [&] {
      return ImGui::SliderFloat("##matAo", &ao, 0.0f, 1.0f);
    })) {
      mtl.SetAo(ao, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Albedo Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> albedoMapPicker;
    if (auto albedoMap{mtl.GetAlbedoMap()}; ImGuiDisabled(!allow_edit, [&] {
      return albedoMapPicker.Draw(albedoMap);
    })) {
      mtl.SetAlbedoMap(albedoMap, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Metallic Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> metallicMapPicker;
    if (auto metallicMap{mtl.GetMetallicMap()}; ImGuiDisabled(!allow_edit, [&] {
      return metallicMapPicker.Draw(metallicMap);
    })) {
      mtl.SetMetallicMap(metallicMap, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Roughness Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> roughnessMapPicker;
    if (auto roughnessMap{mtl.GetRoughnessMap()}; ImGuiDisabled(!allow_edit, [&] {
      return roughnessMapPicker.Draw(roughnessMap);
    })) {
      mtl.SetRoughnessMap(roughnessMap, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Ambient Occlusion Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> aoMapPicker;
    if (auto aoMap{mtl.GetAoMap()}; ImGuiDisabled(!allow_edit, [&] {
      return aoMapPicker.Draw(aoMap);
    })) {
      mtl.SetAoMap(aoMap, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Normal Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> normalMapPicker;
    if (auto normalMap{mtl.GetNormalMap()}; ImGuiDisabled(!allow_edit, [&] {
      return normalMapPicker.Draw(normalMap);
    })) {
      mtl.SetNormalMap(normalMap, GpuResidencyPolicy::kMakeResident);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Blend Mode");
    ImGui::TableNextColumn();
    ImGui::BeginDisabled(!allow_edit);
    if (char const* blendModeNames[]{"Opaque", "Alpha Clipping"}; ImGui::BeginCombo("##blendMode",
      blendModeNames[static_cast<int>(mtl.GetBlendMode())])) {
      for (auto i = 0; i < 2; i++) {
        if (ImGui::Selectable(blendModeNames[i], i == static_cast<int>(mtl.GetBlendMode()))) {
          mtl.SetBlendMode(static_cast<MaterialBlendMode>(i), GpuResidencyPolicy::kMakeResident);
          changed = true;
        }
      }
      ImGui::EndCombo();
    }
    ImGui::EndDisabled();

    if (mtl.GetBlendMode() == MaterialBlendMode::kAlphaClip) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", "Alpha Threshold");
      ImGui::TableNextColumn();
      if (auto thresh{mtl.GetAlphaThreshold()}; ImGuiDisabled(!allow_edit, [&] {
        return ImGui::SliderFloat("##AlphaThresh", &thresh, 0, 1);
      })) {
        mtl.SetAlphaThreshold(thresh, GpuResidencyPolicy::kMakeResident);
        changed = true;
      }

      ImGui::TableNextColumn();
      ImGui::Text("%s", "Opacity Mask");
      ImGui::TableNextColumn();
      static ObjectPicker<Texture2D> opacityMaskPicker;
      if (auto opacityMask{mtl.GetOpacityMask()}; ImGuiDisabled(!allow_edit, [&] {
        return opacityMaskPicker.Draw(opacityMask);
      })) {
        mtl.SetOpacityMask(opacityMask, GpuResidencyPolicy::kMakeResident);
        changed = true;
      }
    }

    ImGui::EndTable();
  }
}
}
