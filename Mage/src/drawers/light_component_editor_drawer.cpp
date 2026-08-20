#include "light_component_editor_drawer.hpp"

#include "editor_drawer_registry.hpp"
#include "gui_helpers.hpp"


namespace sorcery::mage {
auto LightComponentEditorDrawer::Draw(
  EditorDrawerContext const& ctx,
  LightComponent& obj,
  bool const allow_edit,
  bool& changed
) -> void {
  ctx.registry->DrawAs<Component>(obj, allow_edit, changed);

  ImGui::Text("Color");
  ImGui::TableNextColumn();

  Vector3 color{obj.GetColor()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::ColorEdit3("###lightColor", color.GetData());
  })) {
    obj.SetColor(color);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Intensity");
  ImGui::TableNextColumn();

  auto intensity{obj.GetIntensity()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::DragFloat("###lightIntensity", &intensity, 0.1f, LightComponent::MIN_INTENSITY,
      std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp);
  })) {
    obj.SetIntensity(intensity);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Casts Shadow");
  ImGui::TableNextColumn();

  auto castsShadow{obj.IsCastingShadow()};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::Checkbox("###lightCastsShadow", &castsShadow);
  })) {
    obj.SetCastingShadow(castsShadow);
  }

  if (obj.IsCastingShadow()) {
    ImGui::TableNextColumn();

    if (obj.GetType() == LightComponent::Type::Directional) {
      ImGui::Text("%s", "Shadow Extension");
      ImGui::TableNextColumn();

      float shadowExt{obj.GetShadowExtension()};
      if (ImGuiDisabled(!allow_edit, [&] {
        return ImGui::DragFloat("##lightShadowExt", &shadowExt, 1.0f, LightComponent::MIN_SHADOW_EXTENSION,
          std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp);
      })) {
        obj.SetShadowExtension(shadowExt);
      }
    } else {
      ImGui::Text("%s", "Shadow Near Plane");
      ImGui::TableNextColumn();

      auto shadowNearPlane{obj.GetShadowNearPlane()};
      if (ImGuiDisabled(!allow_edit, [&] {
        return ImGui::DragFloat("###lightShadowNearPlane", &shadowNearPlane, 0.01f,
          LightComponent::MIN_SHADOW_NEAR_PLANE,
          std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp);
      })) {
        obj.SetShadowNearPlane(shadowNearPlane);
      }
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Shadow Depth Bias");
    ImGui::TableNextColumn();

    auto shadowDepthBias{obj.GetShadowDepthBias()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::DragFloat("###lightShadowDephBias", &shadowDepthBias, 0.25f, 0, FLT_MAX, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    })) {
      obj.SetShadowDepthBias(shadowDepthBias);
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Shadow Normal Bias");
    ImGui::TableNextColumn();

    auto shadowNormalBias{obj.GetShadowNormalBias()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::DragFloat("###lightShadowNormalBias", &shadowNormalBias, 0.25f, 0, FLT_MAX, "%.3f",
        ImGuiSliderFlags_AlwaysClamp);
    })) {
      obj.SetShadowNormalBias(shadowNormalBias);
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Type");
  ImGui::TableNextColumn();

  constexpr char const* typeOptions[]{"Directional", "Spot", "Point"};
  auto selection{static_cast<int>(obj.GetType())};
  if (ImGuiDisabled(!allow_edit, [&] {
    return ImGui::Combo("###LightType", &selection, typeOptions, 3);
  })) {
    obj.SetType(static_cast<LightComponent::Type>(selection));
  }

  if (obj.GetType() == LightComponent::Type::Spot || obj.GetType() == LightComponent::Type::Point) {
    ImGui::TableNextColumn();
    ImGui::Text("Range");
    ImGui::TableNextColumn();

    auto range{obj.GetRange()};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::DragFloat("###lightRange", &range, 1.0f, LightComponent::MIN_RANGE,
        std::numeric_limits<float>::max(),
        "%.3f", ImGuiSliderFlags_AlwaysClamp);
    })) {
      obj.SetRange(range);
    }
  }

  if (obj.GetType() == LightComponent::Type::Spot) {
    ImGui::TableNextColumn();
    ImGui::Text("Inner Angle");
    ImGui::TableNextColumn();

    auto innerAngleRad{ToRadians(obj.GetInnerAngle())};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::SliderAngle("###spotLightInnerAngle", &innerAngleRad, LightComponent::MIN_ANGLE_DEG,
        LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    })) {
      obj.SetInnerAngle(ToDegrees(innerAngleRad));
    }

    ImGui::TableNextColumn();
    ImGui::Text("Outer Angle");
    ImGui::TableNextColumn();

    auto outerAngleRad{ToRadians(obj.GetOuterAngle())};
    if (ImGuiDisabled(!allow_edit, [&] {
      return ImGui::SliderAngle("###spotLightOuterAngle", &outerAngleRad, LightComponent::MIN_ANGLE_DEG,
        LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    })) {
      obj.SetOuterAngle(ToDegrees(outerAngleRad));
    }
  }
}
}
