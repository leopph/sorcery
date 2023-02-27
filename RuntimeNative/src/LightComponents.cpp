#include "LightComponents.hpp"

#include "Serialization.hpp"

#include <format>
#include <iostream>

#include "Systems.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"

namespace leopph {
Object::Type const LightComponent::SerializationType{ Object::Type::Light };

auto LightComponent::GetSerializationType() const -> Object::Type {
	return SerializationType;
}

auto LightComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Light");
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

auto LightComponent::SerializeTextual(YAML::Node& node) const -> void {
	node["color"] = GetColor();
	node["intensity"] = GetIntensity();
}


auto LightComponent::DeserializeTextual(YAML::Node const& node) -> void {
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

LightComponent::DirectionalLightInfo::DirectionalLightInfo(LightComponent const& lightComponent) :
	mOwningLightComponent{ lightComponent } { }

auto LightComponent::DirectionalLightInfo::GetDirection() const -> Vector3 const& {
	return mOwningLightComponent.GetEntity()->GetTransform().GetForwardAxis();
}

auto LightComponent::DirectionalLightInfo::GetShadowNearPlane() const -> f32 {
	return mShadowNear;
}

auto LightComponent::DirectionalLightInfo::SetShadowNearPlane(f32 const nearPlane) -> void {
	mShadowNear = nearPlane;
}

auto LightComponent::AttenuatedLightInfo::GetRange() const -> f32 {
	return mRange;
}

auto LightComponent::AttenuatedLightInfo::SetRange(f32 const range) -> void {
	mRange = range;
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
}
}
