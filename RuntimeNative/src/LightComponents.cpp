#include "LightComponents.hpp"

#include "Serialization.hpp"

#include "Renderer.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"

#include <algorithm>
#include <cmath>


namespace leopph {
Object::Type const LightComponent::SerializationType{ Object::Type::Light };


LightComponent::LightComponent() {
	renderer::RegisterLight(this);
}


LightComponent::~LightComponent() {
	renderer::UnregisterLight(this);
}


auto LightComponent::GetSerializationType() const -> Object::Type {
	return SerializationType;
}


auto LightComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Light");
}


auto LightComponent::Serialize(YAML::Node& node) const -> void {
	Component::Serialize(node);
	node["color"] = GetColor();
	node["intensity"] = GetIntensity();
	node["type"] = static_cast<int>(GetType());
	node["castsShadow"] = IsCastingShadow();
	node["shadowNearPlane"] = GetShadowNearPlane();
	node["shadowNormalBias"] = GetShadowNormalBias();
	node["range"] = GetRange();
	node["innerAngle"] = GetInnerAngle();
	node["outerAngle"] = GetOuterAngle();
	node["shadowDepthBias"] = GetShadowDepthBias();
	node["shadowExtension"] = GetShadowExtension();
}


auto LightComponent::Deserialize(YAML::Node const& node) -> void {
	Component::Deserialize(node);
	if (auto const data{ node["color"] }) {
		SetColor(data.as<Vector3>(GetColor()));
	}
	if (auto const data{ node["intensity"] }) {
		SetIntensity(data.as<f32>(GetIntensity()));
	}
	if (auto const data{ node["type"] }) {
		SetType(static_cast<Type>(data.as<int>(static_cast<int>(GetType()))));
	}
	if (auto const data{ node["castsShadow"] }) {
		SetCastingShadow(data.as<bool>(IsCastingShadow()));
	}
	if (auto const data{ node["shadowNearPlane"] }) {
		SetShadowNearPlane(data.as<f32>(GetShadowNearPlane()));
	}
	if (auto const data{ node["shadowNormalBias"] }) {
		SetShadowNormalBias(data.as<float>(GetShadowNormalBias()));
	}
	if (auto const data{ node["range"] }) {
		SetRange(data.as<f32>(GetRange()));
	}
	if (auto const data{ node["innerAngle"] }) {
		SetInnerAngle(data.as<f32>(GetInnerAngle()));
	}
	if (auto const data{ node["outerAngle"] }) {
		SetOuterAngle(data.as<f32>(GetOuterAngle()));
	}
	if (auto const data{ node["shadowDepthBias"] }) {
		SetShadowDepthBias(data.as<float>(GetShadowDepthBias()));
	}
	if (auto const data{ node["shadowExtension"] }) {
		SetShadowExtension(data.as<float>(GetShadowExtension()));
	}
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


namespace managedbindings {
auto GetLightColor(MonoObject* light) -> Vector3 {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetColor();
}


auto SetLightColor(MonoObject* light, Vector3 color) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetColor(color);
}


auto GetLightIntensity(MonoObject* light) -> f32 {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetIntensity();
}


auto SetLightIntensity(MonoObject* light, f32 intensity) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetIntensity(intensity);
}


auto GetLightShadowCast(MonoObject* light) -> int {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->IsCastingShadow();
}


auto SetLightShadowCast(MonoObject* light, int const cast) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetCastingShadow(cast);
}


auto GetLightType(MonoObject* light) -> LightComponent::Type {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetType();
}


auto SetLightType(MonoObject* light, LightComponent::Type const type) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetType(type);
}


auto GetLightShadowNearPlane(MonoObject* light) -> float {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetShadowNearPlane();
}


auto SetLightShadowNearPlane(MonoObject* light, float const nearPlane) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetShadowNearPlane(nearPlane);
}


auto GetLightRange(MonoObject* light) -> float {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetRange();
}


auto SetLightRange(MonoObject* light, float const range) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetRange(range);
}


auto GetLightInnerAngle(MonoObject* light) -> float {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetInnerAngle();
}


auto SetLightInnerAngle(MonoObject* light, float const innerAngle) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetInnerAngle(innerAngle);
}


auto GetLightOuterAngle(MonoObject* light) -> float {
	return ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->GetOuterAngle();
}


auto SetLightOuterAngle(MonoObject* light, float const outerAngle) -> void {
	ManagedAccessObject::GetNativePtrFromManagedObjectAs<LightComponent*>(light)->SetOuterAngle(outerAngle);
}
}
}
