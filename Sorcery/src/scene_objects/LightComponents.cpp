#include "LightComponents.hpp"

#include <algorithm>
#include <cmath>

#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "../app.hpp"
#include "../Color.hpp"
#include "../rendering/scene_renderer.hpp"


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
auto LightComponent::OnDrawGizmosSelected() -> void {
  Component::OnDrawGizmosSelected();

  if (GetType() == Type::Spot) {
    auto const modelMtxNoScale{GetEntity()->GetTransform().CalculateLocalToWorldMatrixWithoutScale()};
    auto vertices{CalculateSpotLightLocalVertices(GetRange(), GetOuterAngle())};

    for (auto& vertex : vertices) {
      vertex = Vector3{Vector4{vertex, 1} * modelMtxNoScale};
    }

    Color const lineColor{Color::Magenta()};

    // This highly depends on the order CalculateSpotLightLocalVertices returns the vertices
    for (auto i = 0; i < 4; i++) {
      App::Instance().GetSceneRenderer().DrawLineAtNextRender(vertices[4], vertices[i], lineColor);
      App::Instance().GetSceneRenderer().DrawLineAtNextRender(vertices[i], vertices[(i + 1) % 4], lineColor);
    }
  }
}


auto LightComponent::Clone() -> std::unique_ptr<SceneObject> {
  return Create<LightComponent>(*this);
}


auto LightComponent::OnAfterEnteringScene(Scene const& scene) -> void {
  Component::OnAfterEnteringScene(scene);
  App::Instance().GetSceneRenderer().Register(*this);
}


auto LightComponent::OnBeforeExitingScene(Scene const& scene) -> void {
  App::Instance().GetSceneRenderer().Unregister(*this);
  Component::OnBeforeExitingScene(scene);
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
  return GetEntity()->GetTransform().GetForwardAxis();
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


auto CalculateSpotLightLocalVertices(float const range, float const outer_angle) noexcept -> std::array<Vector3, 5> {
  auto const coneBaseRadius{std::tan(ToRadians(outer_angle / 2.0f)) * range};

  return std::array{
    Vector3{-coneBaseRadius, -coneBaseRadius, range}, Vector3{coneBaseRadius, -coneBaseRadius, range},
    Vector3{coneBaseRadius, coneBaseRadius, range}, Vector3{-coneBaseRadius, coneBaseRadius, range}, Vector3::Zero()
  };
}
}
