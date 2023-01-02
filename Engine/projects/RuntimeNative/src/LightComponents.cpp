#include "LightComponents.hpp"

#include "Serialization.hpp"

#include <format>
#include <imgui.h>
#include <iostream>

#include "Systems.hpp"
#include "Entity.hpp"
#include "TransformComponent.hpp"

namespace leopph {
	auto LightComponent::GetColor() const -> Vector3 const& {
		return mColor;
	}


	auto LightComponent::SetColor(Vector3 const& newColor) -> void {
		mColor = newColor;
	}


	auto LightComponent::GetIntensity() const -> f32 {
		return mIntensity;
	}


	auto LightComponent::SetIntensity(f32 const newIntensity) -> void {
		if (newIntensity <= 0) {
			//Logger::get_instance().warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", newIntensity)); TODO
			return;
		}

		mIntensity = newIntensity;
	}


	auto LightComponent::is_casting_shadow() const -> bool {
		return mCastsShadow;
	}


	auto LightComponent::set_casting_shadow(bool const newValue) -> void {
		mCastsShadow = newValue;
	}


	auto AttenuatedLightComponent::get_range() const -> f32 {
		return mRange;
	}


	auto AttenuatedLightComponent::set_range(f32 const value) -> void {
		mRange = value;
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


	auto DirectionalLightComponent::get_direction() const -> Vector3 const& {
		return GetEntity()->GetTransform().GetForwardAxis();
	}


	auto DirectionalLightComponent::get_shadow_near_plane() const -> f32 {
		return mShadowNear;
	}


	auto DirectionalLightComponent::set_shadow_near_plane(f32 const newVal) -> void {
		mShadowNear = newVal;
	}


	DirectionalLightComponent::DirectionalLightComponent() {
		gRenderer.RegisterDirLight(this);
	}


	DirectionalLightComponent::~DirectionalLightComponent() {
		gRenderer.UnregisterDirLight(this);
	}


	PointLightComponent::PointLightComponent() {
		gRenderer.RegisterPointLight(this);
	}


	PointLightComponent::~PointLightComponent() {
		gRenderer.UnregisterPointLight(this);
	}


	auto SpotLight::get_inner_angle() const -> f32 {
		return mInnerAngle;
	}


	auto SpotLight::set_inner_angle(f32 const degrees) -> void {
		mInnerAngle = degrees;
	}


	auto SpotLight::get_outer_angle() const -> f32 {
		return mOuterAngle;
	}


	auto SpotLight::set_outer_angle(f32 const degrees) -> void {
		mOuterAngle = degrees;
	}


	SpotLight::SpotLight() {
		gRenderer.RegisterSpotLight(this);
	}


	SpotLight::~SpotLight() {
		gRenderer.UnregisterSpotLight(this);
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

	auto DirectionalLightComponent::GetSerializationType() const -> Type {
		return Type::DirectionalLight;
	}

	Object::Type const DirectionalLightComponent::SerializationType{ Type::DirectionalLight };

	auto DirectionalLightComponent::CreateManagedObject() -> MonoObject* {
		return ManagedAccessObject::CreateManagedObject("leopph", "DirectionalLight");
	}

	auto LightComponent::OnGui() -> void {
		if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::PushItemWidth(FLT_MIN);
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-FLT_MIN);

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Color");
			ImGui::TableNextColumn();

			Vector3 color{ mColor };
			if (ImGui::ColorEdit3("###lightColor", color.get_data())) {
				SetColor(color);
			}

			ImGui::TableNextColumn();
			ImGui::Text("Intensity");
			ImGui::TableNextColumn();

			auto intensity{ mIntensity };
			if (ImGui::DragFloat("###lightIntensity", &intensity, 0.1f)) {
				SetIntensity(intensity);
			}

			ImGui::EndTable();
		}
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
