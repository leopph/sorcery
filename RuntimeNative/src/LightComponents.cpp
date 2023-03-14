#include "LightComponents.hpp"

#include "Serialization.hpp"

#include <iostream>

#include "Systems.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"

namespace leopph {
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

auto LightComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Light");
}

auto LightComponent::Serialize(YAML::Node& node) const -> void {
	node["color"] = GetColor();
	node["intensity"] = GetIntensity();
}


auto LightComponent::Deserialize(YAML::Node const& node) -> void {
	if (node["color"]) {
		if (!node["color"].IsSequence()) {
			std::cerr << "Failed to deserialize color of LightComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetColor(node.as<Vector3>(GetColor()));
		}
	}
	if (node["intensity"]) {
		if (!node["intensity"].IsScalar()) {
			std::cerr << "Failed to deserialize intensity of LightComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetIntensity(node.as<f32>(GetIntensity()));
		}
	}
}

auto LightComponent::GetColor() const -> Vector3 const& {
	return mColor;
}


auto LightComponent::SetColor(Vector3 const& color) -> void {
	mColor = color;
}


auto LightComponent::GetIntensity() const -> f32 {
	return mIntensity;
}


auto LightComponent::SetIntensity(f32 const intensity) -> void {
	if (intensity <= 0) {
		//Logger::get_instance().warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", intensity)); TODO
		return;
	}

	mIntensity = intensity;
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
	mShadowNear = nearPlane;
}

auto LightComponent::GetRange() const -> f32 {
	return mRange;
}

auto LightComponent::SetRange(f32 const range) -> void {
	mRange = range;
}

auto LightComponent::GetInnerAngle() const -> f32 {
	return mInnerAngle;
}

auto LightComponent::SetInnerAngle(f32 const degrees) -> void {
	mInnerAngle = degrees;
}

auto LightComponent::GetOuterAngle() const -> f32 {
	return mOuterAngle;
}

auto LightComponent::SetOuterAngle(f32 const degrees) -> void {
	mOuterAngle = degrees;
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
