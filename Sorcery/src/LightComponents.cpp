#include "LightComponents.hpp"

#include "Renderer.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"
#include "Systems.hpp"

#include <algorithm>
#include <cmath>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::LightComponent>{ "Light Component" }
    .REFLECT_REGISTER_COMPONENT_CTOR
    .property("color", &sorcery::LightComponent::GetColor, &sorcery::LightComponent::SetColor)
    .property("intensity", &sorcery::LightComponent::GetIntensity, &sorcery::LightComponent::SetIntensity)
    .property("type", &sorcery::LightComponent::GetType, &sorcery::LightComponent::SetType)
    .property("castsShadow", &sorcery::LightComponent::IsCastingShadow, &sorcery::LightComponent::SetCastingShadow)
    .property("shadowNearClipPlane", &sorcery::LightComponent::GetShadowNearPlane, &sorcery::LightComponent::SetShadowNearPlane)
    .property("shadowNormalBias", &sorcery::LightComponent::GetShadowNormalBias, &sorcery::LightComponent::SetShadowNormalBias)
    .property("shadowDepthBias", &sorcery::LightComponent::GetShadowDepthBias, &sorcery::LightComponent::SetShadowDepthBias)
    .property("shadowExtension", &sorcery::LightComponent::GetShadowExtension, &sorcery::LightComponent::SetShadowExtension)
    .property("range", &sorcery::LightComponent::GetRange, &sorcery::LightComponent::SetRange)
    .property("innerAngle", &sorcery::LightComponent::GetInnerAngle, &sorcery::LightComponent::SetInnerAngle)
    .property("outerAngle", &sorcery::LightComponent::GetOuterAngle, &sorcery::LightComponent::SetOuterAngle);
}


namespace sorcery {
Object::Type const LightComponent::SerializationType{ Object::Type::Light };


LightComponent::LightComponent() {
  gRenderer.RegisterLight(this);
}


LightComponent::~LightComponent() {
  gRenderer.UnregisterLight(this);
}


auto LightComponent::GetSerializationType() const -> Object::Type {
  return SerializationType;
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


auto AmbientLight::get_instance() -> AmbientLight& {
  static AmbientLight instance;
  return instance;
}


auto AmbientLight::get_intensity() const -> Vector3 const& {
  return mIntensity;
}


auto AmbientLight::set_intensity(Vector3 const& intensity) -> void {
  mIntensity = intensity;
}


auto CalculateSpotLightLocalVertices(LightComponent const& spotLight) noexcept -> std::array<Vector3, 5> {
  auto const range{ spotLight.GetRange() };
  auto const coneBaseRadius{ std::tan(ToRadians(spotLight.GetOuterAngle() / 2.0f)) * range };

  return std::array{
    Vector3{ -coneBaseRadius, -coneBaseRadius, range },
    Vector3{ coneBaseRadius, -coneBaseRadius, range },
    Vector3{ coneBaseRadius, coneBaseRadius, range },
    Vector3{ -coneBaseRadius, coneBaseRadius, range },
    Vector3::Zero()
  };
}
}
