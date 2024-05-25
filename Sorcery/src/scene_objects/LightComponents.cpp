#include "LightComponents.hpp"

#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "../engine_context.hpp"

#include <algorithm>
#include <cmath>
#include <imgui.h>


RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::LightComponent::Type>("Light Type")(
    rttr::value("Directional", sorcery::LightComponent::Type::Directional),
    rttr::value("Spot", sorcery::LightComponent::Type::Spot),
    rttr::value("Point", sorcery::LightComponent::Type::Point));

  rttr::registration::class_<sorcery::LightComponent>{"Light Component"}.REFLECT_REGISTER_COMPONENT_CTOR.
    property("color", &sorcery::LightComponent::GetColor, &sorcery::LightComponent::SetColor).
    property("intensity", &sorcery::LightComponent::GetIntensity, &sorcery::LightComponent::SetIntensity).
    property("type", &sorcery::LightComponent::GetType, &sorcery::LightComponent::SetType).
    property("castsShadow", &sorcery::LightComponent::IsCastingShadow, &sorcery::LightComponent::SetCastingShadow).
    property("shadowNearClipPlane", &sorcery::LightComponent::GetShadowNearPlane,
      &sorcery::LightComponent::SetShadowNearPlane).
    property("shadowNormalBias", &sorcery::LightComponent::GetShadowNormalBias,
      &sorcery::LightComponent::SetShadowNormalBias).
    property("shadowDepthBias", &sorcery::LightComponent::GetShadowDepthBias,
      &sorcery::LightComponent::SetShadowDepthBias).
    property("shadowExtension", &sorcery::LightComponent::GetShadowExtension,
      &sorcery::LightComponent::SetShadowExtension).
    property("range", &sorcery::LightComponent::GetRange, &sorcery::LightComponent::SetRange).property("innerAngle",
      &sorcery::LightComponent::GetInnerAngle, &sorcery::LightComponent::SetInnerAngle).property("outerAngle",
      &sorcery::LightComponent::GetOuterAngle, &sorcery::LightComponent::SetOuterAngle);
}


namespace sorcery {
LightComponent::~LightComponent() {
  g_engine_context.scene_renderer->Unregister(*this);
}


auto LightComponent::Clone() -> LightComponent* {
  return Create<LightComponent>(*this).release();
}


auto LightComponent::GetColor() const -> Vector3 const& {
  return mColor;
}


auto LightComponent::SetColor(Vector3 const& color) -> void {
  mColor = Clamp(color, 0.0f, 1.0f);
}


auto LightComponent::GetIntensity() const -> f32 {
  return mIntensity;
}


auto LightComponent::SetIntensity(f32 const intensity) -> void {
  mIntensity = std::max(intensity, MIN_INTENSITY);
}


auto LightComponent::IsCastingShadow() const -> bool {
  return mCastsShadow;
}


auto LightComponent::SetCastingShadow(bool const castShadow) -> void {
  mCastsShadow = castShadow;
}


auto LightComponent::GetType() const noexcept -> Type {
  return mType;
}


auto LightComponent::SetType(Type const type) noexcept -> void {
  mType = type;
}


auto LightComponent::GetDirection() const -> Vector3 const& {
  return GetEntity().GetTransform().GetForwardAxis();
}


auto LightComponent::GetShadowNearPlane() const -> f32 {
  return mShadowNear;
}


auto LightComponent::SetShadowNearPlane(f32 const nearPlane) -> void {
  mShadowNear = std::max(nearPlane, MIN_SHADOW_NEAR_PLANE);
}


auto LightComponent::GetRange() const -> f32 {
  return mRange;
}


auto LightComponent::SetRange(f32 const range) -> void {
  mRange = std::max(range, MIN_RANGE);
}


auto LightComponent::GetInnerAngle() const -> f32 {
  return mInnerAngle;
}


auto LightComponent::SetInnerAngle(f32 const degrees) -> void {
  mInnerAngle = std::clamp(degrees, MIN_ANGLE_DEG, GetOuterAngle());
}


auto LightComponent::GetOuterAngle() const -> f32 {
  return mOuterAngle;
}


auto LightComponent::SetOuterAngle(f32 const degrees) -> void {
  mOuterAngle = std::clamp(degrees, GetInnerAngle(), MAX_ANGLE_DEG);
}


auto LightComponent::GetShadowNormalBias() const noexcept -> float {
  return mShadowNormalBias;
}


auto LightComponent::SetShadowNormalBias(float const bias) noexcept -> void {
  mShadowNormalBias = std::max(bias, 0.0f);
}


auto LightComponent::GetShadowDepthBias() const noexcept -> float {
  return mShadowDepthBias;
}


auto LightComponent::SetShadowDepthBias(float const bias) noexcept -> void {
  mShadowDepthBias = bias;
}


auto LightComponent::GetShadowExtension() const noexcept -> float {
  return mShadowExtension;
}


auto LightComponent::SetShadowExtension(float const shadowExtension) noexcept -> void {
  mShadowExtension = std::max(shadowExtension, MIN_SHADOW_EXTENSION);
}


auto LightComponent::Initialize() -> void {
  Component::Initialize();
  g_engine_context.scene_renderer->Register(*this);
}


auto LightComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("Color");
  ImGui::TableNextColumn();

  Vector3 color{GetColor()};
  if (ImGui::ColorEdit3("###lightColor", color.GetData())) {
    SetColor(color);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Intensity");
  ImGui::TableNextColumn();

  auto intensity{GetIntensity()};
  if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f, LightComponent::MIN_INTENSITY,
    std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
    SetIntensity(intensity);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Casts Shadow");
  ImGui::TableNextColumn();

  auto castsShadow{IsCastingShadow()};
  if (ImGui::Checkbox("###lightCastsShadow", &castsShadow)) {
    SetCastingShadow(castsShadow);
  }

  if (IsCastingShadow()) {
    ImGui::TableNextColumn();

    if (GetType() == LightComponent::Type::Directional) {
      ImGui::Text("%s", "Shadow Extension");
      ImGui::TableNextColumn();

      float shadowExt{GetShadowExtension()};
      if (ImGui::DragFloat("##lightShadowExt", &shadowExt, 1.0f, LightComponent::MIN_SHADOW_EXTENSION,
        std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        SetShadowExtension(shadowExt);
      }
    } else {
      ImGui::Text("%s", "Shadow Near Plane");
      ImGui::TableNextColumn();

      auto shadowNearPlane{GetShadowNearPlane()};
      if (ImGui::DragFloat("###lightShadowNearPlane", &shadowNearPlane, 0.01f, LightComponent::MIN_SHADOW_NEAR_PLANE,
        std::numeric_limits<float>::max(), "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
        SetShadowNearPlane(shadowNearPlane);
      }
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Shadow Depth Bias");
    ImGui::TableNextColumn();

    auto shadowDepthBias{GetShadowDepthBias()};
    if (ImGui::DragFloat("###lightShadowDephBias", &shadowDepthBias, 0.25f, 0, FLT_MAX, "%.3f",
      ImGuiSliderFlags_AlwaysClamp)) {
      SetShadowDepthBias(shadowDepthBias);
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Shadow Normal Bias");
    ImGui::TableNextColumn();

    auto shadowNormalBias{GetShadowNormalBias()};
    if (ImGui::DragFloat("###lightShadowNormalBias", &shadowNormalBias, 0.25f, 0, FLT_MAX, "%.3f",
      ImGuiSliderFlags_AlwaysClamp)) {
      SetShadowNormalBias(shadowNormalBias);
    }
  }

  ImGui::TableNextColumn();
  ImGui::Text("Type");
  ImGui::TableNextColumn();

  constexpr char const* typeOptions[]{"Directional", "Spot", "Point"};
  int selection{static_cast<int>(GetType())};
  if (ImGui::Combo("###LightType", &selection, typeOptions, 3)) {
    SetType(static_cast<LightComponent::Type>(selection));
  }

  if (GetType() == LightComponent::Type::Spot || GetType() == LightComponent::Type::Point) {
    ImGui::TableNextColumn();
    ImGui::Text("Range");
    ImGui::TableNextColumn();

    auto range{GetRange()};
    if (ImGui::DragFloat("###lightRange", &range, 1.0f, LightComponent::MIN_RANGE, std::numeric_limits<float>::max(),
      "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      SetRange(range);
    }
  }

  if (GetType() == LightComponent::Type::Spot) {
    ImGui::TableNextColumn();
    ImGui::Text("Inner Angle");
    ImGui::TableNextColumn();

    auto innerAngleRad{ToRadians(GetInnerAngle())};
    if (ImGui::SliderAngle("###spotLightInnerAngle", &innerAngleRad, LightComponent::MIN_ANGLE_DEG,
      LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      SetInnerAngle(ToDegrees(innerAngleRad));
    }

    ImGui::TableNextColumn();
    ImGui::Text("Outer Angle");
    ImGui::TableNextColumn();

    auto outerAngleRad{ToRadians(GetOuterAngle())};
    if (ImGui::SliderAngle("###spotLightOuterAngle", &outerAngleRad, LightComponent::MIN_ANGLE_DEG,
      LightComponent::MAX_ANGLE_DEG, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
      SetOuterAngle(ToDegrees(outerAngleRad));
    }
  }
}


auto LightComponent::OnDrawGizmosSelected() -> void {
  Component::OnDrawGizmosSelected();

  if (GetType() == Type::Spot) {
    auto const modelMtxNoScale{GetEntity().GetTransform().CalculateLocalToWorldMatrixWithoutScale()};
    auto vertices{CalculateSpotLightLocalVertices(GetRange(), GetOuterAngle())};

    for (auto& vertex : vertices) {
      vertex = Vector3{Vector4{vertex, 1} * modelMtxNoScale};
    }

    Color const lineColor{Color::Magenta()};

    // This highly depends on the order CalculateSpotLightLocalVertices returns the vertices
    for (int i = 0; i < 4; i++) {
      g_engine_context.scene_renderer->DrawLineAtNextRender(vertices[4], vertices[i], lineColor);
      g_engine_context.scene_renderer->DrawLineAtNextRender(vertices[i], vertices[(i + 1) % 4], lineColor);
    }
  }
}


auto CalculateSpotLightLocalVertices(float const range, float const outer_angle) noexcept -> std::array<Vector3, 5> {
  auto const coneBaseRadius{std::tan(ToRadians(outer_angle / 2.0f)) * range};

  return std::array{
    Vector3{-coneBaseRadius, -coneBaseRadius, range}, Vector3{coneBaseRadius, -coneBaseRadius, range},
    Vector3{coneBaseRadius, coneBaseRadius, range}, Vector3{-coneBaseRadius, coneBaseRadius, range}, Vector3::Zero()
  };
}
}
