#include "material_editor_drawer.hpp"

#include <functional>
#include <limits>

#include <imgui.h>

#include "editor_drawer_registry.hpp"
#include "../gui_helpers.hpp"


namespace sorcery::mage {
namespace {
template<typename F>
auto DrawSection(char const* const name, F&& draw_rows) -> void {
  ImGui::SeparatorText(name);

  if (ImGui::BeginTable(name, 2, ImGuiTableFlags_SizingStretchSame)) {
    std::invoke(std::forward<F>(draw_rows));
    ImGui::EndTable();
  }
}


// Returns whether the value was edited.
template<typename F>
auto DrawRow(char const* const label, bool const allow_edit, F&& draw_widget) -> bool {
  ImGui::TableNextColumn();
  ImGui::TextUnformatted(label);
  ImGui::TableNextColumn();

  // Rows share widget IDs, so scope them by label
  ImGui::PushID(label);
  ImGui::PushItemWidth(-FLT_MIN);
  auto const edited = ImGuiDisabled(!allow_edit, std::forward<F>(draw_widget));
  ImGui::PopItemWidth();
  ImGui::PopID();
  return edited;
}
}


auto MaterialEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  Material& mtl,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<NativeAsset>(mtl, allow_edit, changed);

  auto const draw_map_row = [&](
    char const* const label,
    ObjectPicker<Texture2D>& picker,
    AssetRef<Texture2D> (Material::*const getter)() const,
    void (Material::*const setter)(AssetRef<Texture2D> const&)
  ) {
    if (auto map{(mtl.*getter)().Get()}; DrawRow(label, allow_edit, [&] {
      return picker.Draw(map);
    })) {
      (mtl.*setter)(MakeAssetRef(map.Get()));
      changed = true;
    }
  };

  auto const draw_unorm_row = [&](
    char const* const label,
    f32 (Material::*const getter)() const,
    void (Material::*const setter)(f32)
  ) {
    if (auto value{(mtl.*getter)()}; DrawRow(label, allow_edit, [&] {
      return ImGui::SliderFloat("##value", &value, 0.0f, 1.0f);
    })) {
      (mtl.*setter)(value);
      changed = true;
    }
  };

  ImGui::PushID(mtl.GetAssetId().GetGuid().ToString().c_str());

  DrawSection("Base Color", [&] {
    if (Vector3 base_color{mtl.GetBaseColorVector()}; DrawRow("Color", allow_edit, [&] {
      return ImGui::ColorEdit3("##value", base_color.GetData());
    })) {
      mtl.SetBaseColorVector(base_color);
      changed = true;
    }

    static ObjectPicker<Texture2D> base_color_map_picker;
    draw_map_row("Map", base_color_map_picker, &Material::GetBaseColorMap, &Material::SetBaseColorMap);
  });

  DrawSection("Metallic", [&] {
    draw_unorm_row("Value", &Material::GetMetallic, &Material::SetMetallic);
    static ObjectPicker<Texture2D> metallic_map_picker;
    draw_map_row("Map", metallic_map_picker, &Material::GetMetallicMap, &Material::SetMetallicMap);
  });

  DrawSection("Roughness", [&] {
    draw_unorm_row("Value", &Material::GetRoughness, &Material::SetRoughness);
    static ObjectPicker<Texture2D> roughness_map_picker;
    draw_map_row("Map", roughness_map_picker, &Material::GetRoughnessMap, &Material::SetRoughnessMap);
  });

  DrawSection("Ambient Occlusion", [&] {
    draw_unorm_row("Value", &Material::GetAo, &Material::SetAo);
    static ObjectPicker<Texture2D> ao_map_picker;
    draw_map_row("Map", ao_map_picker, &Material::GetAoMap, &Material::SetAoMap);
  });

  DrawSection("Emissive", [&] {
    if (Vector3 emissive{mtl.GetEmissiveColorVector()}; DrawRow("Color", allow_edit, [&] {
      return ImGui::ColorEdit3("##value", emissive.GetData());
    })) {
      mtl.SetEmissiveColorVector(emissive);
      changed = true;
    }

    if (auto intensity{mtl.GetEmissiveIntensity()}; DrawRow("Intensity", allow_edit, [&] {
      return ImGui::DragFloat("##value", &intensity, 0.01f, 0.0f, std::numeric_limits<float>::max());
    })) {
      mtl.SetEmissiveIntensity(intensity);
      changed = true;
    }

    static ObjectPicker<Texture2D> emissive_map_picker;
    draw_map_row("Map", emissive_map_picker, &Material::GetEmissiveMap, &Material::SetEmissiveMap);
  });

  DrawSection("Normal", [&] {
    static ObjectPicker<Texture2D> normal_map_picker;
    draw_map_row("Map", normal_map_picker, &Material::GetNormalMap, &Material::SetNormalMap);
  });

  DrawSection("Blending", [&] {
    if (auto blend_mode{static_cast<int>(mtl.GetBlendMode())}; DrawRow("Blend Mode", allow_edit, [&] {
      char const* const blend_mode_names[]{"Opaque", "Alpha Clipping"};
      return ImGui::Combo("##value", &blend_mode, blend_mode_names, IM_ARRAYSIZE(blend_mode_names));
    })) {
      mtl.SetBlendMode(static_cast<MaterialBlendMode>(blend_mode));
      changed = true;
    }

    if (mtl.GetBlendMode() == MaterialBlendMode::kAlphaClip) {
      if (auto thresh{mtl.GetAlphaThreshold()}; DrawRow("Alpha Threshold", allow_edit, [&] {
        return ImGui::SliderFloat("##value", &thresh, 0.0f, 1.0f);
      })) {
        mtl.SetAlphaThreshold(thresh);
        changed = true;
      }

      draw_unorm_row("Opacity", &Material::GetOpacity, &Material::SetOpacity);

      static ObjectPicker<Texture2D> opacity_mask_picker;
      draw_map_row("Opacity Mask", opacity_mask_picker, &Material::GetOpacityMap, &Material::SetOpacityMap);

      if (auto channel = static_cast<int>(mtl.GetOpacityChannel()); DrawRow("Opacity Mask Channel", allow_edit, [&] {
        char const* const channels[]{"R", "G", "B", "A"};
        return ImGui::Combo("##value", &channel, channels, IM_ARRAYSIZE(channels));
      })) {
        mtl.SetOpacityChannel(static_cast<std::uint32_t>(channel));
        changed = true;
      }
    }
  });

  ImGui::PopID();
}
}
