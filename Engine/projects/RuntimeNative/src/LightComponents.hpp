#pragma once

#include "Component.hpp"
#include "Math.hpp"

namespace leopph {
	class LightComponent : public Component {
	public:
		[[nodiscard]] LEOPPHAPI auto GetColor() const -> Vector3 const&;
		LEOPPHAPI auto SetColor(Vector3 const& newColor) -> void;

		[[nodiscard]] LEOPPHAPI auto GetIntensity() const -> f32;
		LEOPPHAPI auto SetIntensity(f32 newIntensity) -> void;

		[[nodiscard]] LEOPPHAPI auto is_casting_shadow() const -> bool;
		LEOPPHAPI auto set_casting_shadow(bool newValue) -> void;

		LEOPPHAPI auto OnGui() -> void override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

	protected:
		LightComponent() = default;
		LightComponent(LightComponent const& other) = default;
		LightComponent(LightComponent&& other) noexcept = default;

	public:
		~LightComponent() override = default;

	protected:
		auto operator=(LightComponent const& other) -> LightComponent& = default;
		auto operator=(LightComponent&& other) noexcept -> LightComponent& = default;

	private:
		bool mCastsShadow{ false };
		Vector3 mColor{ 1.f };
		f32 mIntensity{ 1.f };
	};


	class AttenuatedLightComponent : public LightComponent {
	public:
		[[nodiscard]] LEOPPHAPI auto get_range() const -> f32;
		LEOPPHAPI auto set_range(f32 value) -> void;

	protected:
		AttenuatedLightComponent() = default;
		AttenuatedLightComponent(AttenuatedLightComponent const& other) = default;
		AttenuatedLightComponent(AttenuatedLightComponent&& other) noexcept = default;

	public:
		~AttenuatedLightComponent() override = default;

	protected:
		auto operator=(AttenuatedLightComponent const& other) -> AttenuatedLightComponent& = default;
		auto operator=(AttenuatedLightComponent&& other) noexcept -> AttenuatedLightComponent& = default;

	private:
		f32 mRange{ 10.f };
	};


	class AmbientLight final {
	public:
		LEOPPHAPI static auto get_instance() -> AmbientLight&;

		[[nodiscard]] LEOPPHAPI auto get_intensity() const -> Vector3 const&;
		LEOPPHAPI auto set_intensity(Vector3 const& intensity) -> void;

	private:
		AmbientLight() = default;

	public:
		AmbientLight(AmbientLight const& other) = delete;
		AmbientLight(AmbientLight&& other) = delete;

	private:
		~AmbientLight() = default;

	public:
		auto operator=(AmbientLight const& other) -> AmbientLight& = delete;
		auto operator=(AmbientLight&& other) -> AmbientLight& = delete;

	private:
		Vector3 mIntensity{ 0.1f, 0.1f, 0.1f };
	};


	class DirectionalLightComponent final : public LightComponent {
	public:
		[[nodiscard]] LEOPPHAPI auto get_direction() const -> Vector3 const&;

		[[nodiscard]] LEOPPHAPI auto get_shadow_near_plane() const -> f32;
		LEOPPHAPI auto set_shadow_near_plane(f32 newVal) -> void;

		LEOPPHAPI DirectionalLightComponent();
		LEOPPHAPI DirectionalLightComponent(DirectionalLightComponent const& other) = delete;
		LEOPPHAPI DirectionalLightComponent(DirectionalLightComponent&& other) noexcept = delete;

		LEOPPHAPI ~DirectionalLightComponent() override;

		LEOPPHAPI auto operator=(DirectionalLightComponent const& other) -> DirectionalLightComponent&;
		LEOPPHAPI auto operator=(DirectionalLightComponent&& other) noexcept -> DirectionalLightComponent&;

		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

	private:
		f32 mShadowNear{ 50.f };
	};


	class PointLightComponent final : public AttenuatedLightComponent {
	public:
		LEOPPHAPI PointLightComponent();
		LEOPPHAPI PointLightComponent(PointLightComponent const& other);
		LEOPPHAPI PointLightComponent(PointLightComponent&& other) noexcept;

		LEOPPHAPI ~PointLightComponent() override;

		LEOPPHAPI auto operator=(PointLightComponent const& other) -> PointLightComponent&;
		LEOPPHAPI auto operator=(PointLightComponent&& other) noexcept -> PointLightComponent&;
	};


	class SpotLight final : public AttenuatedLightComponent {
	public:
		[[nodiscard]] LEOPPHAPI auto get_inner_angle() const -> f32;
		LEOPPHAPI auto set_inner_angle(f32 degrees) -> void;

		[[nodiscard]] LEOPPHAPI auto get_outer_angle() const -> f32;
		LEOPPHAPI auto set_outer_angle(f32 degrees) -> void;

		LEOPPHAPI SpotLight();
		LEOPPHAPI SpotLight(SpotLight const& other);
		LEOPPHAPI SpotLight(SpotLight&& other) noexcept;

		LEOPPHAPI ~SpotLight() override;

		LEOPPHAPI auto operator=(SpotLight const& other) -> SpotLight&;
		LEOPPHAPI auto operator=(SpotLight&& other) noexcept -> SpotLight&;

	private:
		f32 mInnerAngle{ 30.f };
		f32 mOuterAngle{ 30.f };
	};


	namespace managedbindings {
		auto GetLightColor(MonoObject* light) -> Vector3;
		auto SetLightColor(MonoObject* light, Vector3 color) -> void;

		auto GetLightIntensity(MonoObject* light) -> f32;
		auto SetLightIntensity(MonoObject* light, f32 intensity) -> void;
	}
}
