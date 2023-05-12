#include <imgui.h>

#include "ObjectWrappers.hpp"


namespace leopph::editor {
auto ObjectWrapperFor<LightComponent>::OnDrawProperties([[maybe_unused]] Context& context, Object& object) -> void {
  auto& light{ dynamic_cast<LightComponent&>(object) };

  if (ImGui::BeginTable(std::format("{}", light.GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Color");
    ImGui::TableNextColumn();

    Vector3 color{ light.GetColor() };
    if (ImGui::ColorEdit3("###lightColor", color.GetData())) {
      light.SetColor(color);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Intensity");
    ImGui::TableNextColumn();

    auto intensity{ light.GetIntensity() };
    if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f, LightComponent::MIN_INTENSITY, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      light.SetIntensity(intensity);
    }

    ImGui::TableNextColumn();
    ImGui::Text("Casts Shadow");
    ImGui::TableNextColumn();

    auto castsShadow{ light.IsCastingShadow() };
    if (ImGui::Checkbox("###lightCastsShadow", &castsShadow)) {
      light.SetCastingShadow(castsShadow);
    }

    if (light.IsCastingShadow()) {
      ImGui::TableNextColumn();

      if (light.GetType() == LightComponent::Type::Directional) {
        ImGui::Text("%s", "Shadow Extension");
        ImGui::TableNextColumn();

        float shadowExt{ light.GetShadowExtension() };
        if (ImGui::DragFloat("##lightShadowExt", &shadowExt, 1.0f, LightComponent::MIN_SHADOW_EXTENSION, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
          light.SetShadowExtension(shadowExt);
        }
      } else {
        ImGui::Text("%s", "Shadow Near Plane");
        ImGui::TableNextColumn();

        auto shadowNearPlane{ light.GetShadowNearPlane() };
        if (ImGui::DragFloat("###lightShadowNearPlane", &shadowNearPlane, 0.01f, LightComponent::MIN_SHADOW_NEAR_PLANE, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
          light.SetShadowNearPlane(shadowNearPlane);
        }
      }

      ImGui::TableNextColumn();
      ImGui::Text("%s", "Shadow Depth Bias");
      ImGui::TableNextColumn();

      auto shadowDepthBias{ light.GetShadowDepthBias() };
      if (ImGui::DragFloat("###lightShadowDephBias", &shadowDepthBias, 0.25f, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        light.SetShadowDepthBias(shadowDepthBias);
      }

      ImGui::TableNextColumn();
      ImGui::Text("%s", "Shadow Normal Bias");
      ImGui::TableNextColumn();

      auto shadowNormalBias{ light.GetShadowNormalBias() };
      if (ImGui::DragFloat("###lightShadowNormalBias", &shadowNormalBias, 0.25f, 0, FLT_MAX, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        light.SetShadowNormalBias(shadowNormalBias);
      }
    }

    ImGui::TableNextColumn();
    ImGui::Text("Type");
    ImGui::TableNextColumn();

    constexpr char const* typeOptions[]{ "Directional", "Spot", "Point" };
    int selection{ static_cast<int>(light.GetType()) };
    if (ImGui::Combo("###LightType", &selection, typeOptions, 3)) {
      light.SetType(static_cast<LightComponent::Type>(selection));
    }

    if (light.GetType() == LightComponent::Type::Spot || light.GetType() == LightComponent::Type::Point) {
      ImGui::TableNextColumn();
      ImGui::Text("Range");
      ImGui::TableNextColumn();

      auto range{ light.GetRange() };
      if (ImGui::DragFloat("###lightRange", &range, 1.0f, LightComponent::MIN_RANGE, std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        light.SetRange(range);
      }
    }

    if (light.GetType() == LightComponent::Type::Spot) {
      ImGui::TableNextColumn();
      ImGui::Text("Inner Angle");
      ImGui::TableNextColumn();

      auto innerAngleRad{ ToRadians(light.GetInnerAngle()) };
      if (ImGui::SliderAngle("###spotLightInnerAngle", &innerAngleRad, LightComponent::MIN_ANGLE_DEG, LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        light.SetInnerAngle(ToDegrees(innerAngleRad));
      }

      ImGui::TableNextColumn();
      ImGui::Text("Outer Angle");
      ImGui::TableNextColumn();

      auto outerAngleRad{ ToRadians(light.GetOuterAngle()) };
      if (ImGui::SliderAngle("###spotLightOuterAngle", &outerAngleRad, LightComponent::MIN_ANGLE_DEG, LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        light.SetOuterAngle(ToDegrees(outerAngleRad));
      }
    }

    ImGui::EndTable();
  }
}


auto ObjectWrapperFor<LightComponent>::OnDrawGizmosSelected([[maybe_unused]] Context& context, Object& object) -> void {
  auto const& light{ dynamic_cast<LightComponent&>(object) };

  if (light.GetType() == LightComponent::Type::Spot) {
    auto const modelMtxNoScale{ CalculateModelMatrixNoScale(light.GetEntity()->GetTransform()) };
    auto vertices{ CalculateSpotLightLocalVertices(light) };

    for (auto& vertex : vertices) {
      vertex = Vector3{ Vector4{ vertex, 1 } * modelMtxNoScale };
    }

    Color const lineColor{ Color::Magenta() };

    // This highly depends on the order CalculateSpotLightLocalVertices returns the vertices
    for (int i = 0; i < 4; i++) {
      renderer::DrawLineAtNextRender(vertices[4], vertices[i], lineColor);
      renderer::DrawLineAtNextRender(vertices[i], vertices[(i + 1) % 4], lineColor);
    }
  }
}
}
