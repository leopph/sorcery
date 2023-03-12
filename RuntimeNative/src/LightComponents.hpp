#pragma once

#include "Component.hpp"
#include "Math.hpp"

namespace leopph {
class LightComponent : public Component {
public:
	LEOPPHAPI static Type const SerializationType;
	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

	LEOPPHAPI auto CreateManagedObject() -> void override;

	[[nodiscard]] LEOPPHAPI auto GetColor() const -> Vector3 const&;
	LEOPPHAPI auto SetColor(Vector3 const& color) -> void;

	[[nodiscard]] LEOPPHAPI auto GetIntensity() const -> f32;
	LEOPPHAPI auto SetIntensity(f32 intensity) -> void;

	[[nodiscard]] LEOPPHAPI auto IsCastingShadow() const -> bool;
	LEOPPHAPI auto SetCastingShadow(bool castShadow) -> void;

	enum class Type {
		Directional,
		Spot,
		Point
	};

	class DirectionalLightInfo {
		friend class LightComponent;

		f32 mShadowNear{ 50.f };
		LightComponent const& mOwningLightComponent;

		LEOPPHAPI explicit DirectionalLightInfo(LightComponent const& lightComponent);

	public:
		[[nodiscard]] LEOPPHAPI auto GetDirection() const -> Vector3 const&;
		[[nodiscard]] LEOPPHAPI auto GetShadowNearPlane() const -> f32;
		LEOPPHAPI auto SetShadowNearPlane(f32 nearPlane) -> void;
	};

	class AttenuatedLightInfo {
		f32 mRange{ 10.f };

	protected:
		AttenuatedLightInfo() = default;

	public:
		[[nodiscard]] LEOPPHAPI auto GetRange() const -> f32;
		LEOPPHAPI auto SetRange(f32 range) -> void;

		virtual ~AttenuatedLightInfo() = default;
		AttenuatedLightInfo(AttenuatedLightInfo const&) = delete;
		AttenuatedLightInfo(AttenuatedLightInfo&&) = delete;
		void operator=(AttenuatedLightInfo const&) = delete;
		void operator=(AttenuatedLightInfo&&) = delete;
	};

	class SpotLightInfo final : public AttenuatedLightInfo {
		friend class LightComponent;
		SpotLightInfo() = default;

		f32 mInnerAngle{ 30.f };
		f32 mOuterAngle{ 30.f };

	public:
		[[nodiscard]] LEOPPHAPI auto GetInnerAngle() const -> f32;
		LEOPPHAPI auto SetInnerAngle(f32 degrees) -> void;

		[[nodiscard]] LEOPPHAPI auto GetOuterAngle() const -> f32;
		LEOPPHAPI auto SetOuterAngle(f32 degrees) -> void;
	};

	class PointLightInfo final : public AttenuatedLightInfo {
		friend class LightComponent;
		PointLightInfo() = default;
	};

	[[nodiscard]] LEOPPHAPI auto GetType() const noexcept -> Type;

	[[nodiscard]] auto&& GetDirInfo(this auto&& self) {
		return self.mDirInfo;
	}

	[[nodiscard]] auto&& GetSpotInfo(this auto&& self) {
		return self.mSpotInfo;
	}

	[[nodiscard]] auto&& GetPointInfo(this auto&& self) {
		return self.mPointInfo;
	}

	LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
	LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

private:
	bool mCastsShadow{ false };
	Vector3 mColor{ 1.f };
	f32 mIntensity{ 1.f };
	Type mType{ Type::Directional };
	DirectionalLightInfo mDirInfo{ *this };
	SpotLightInfo mSpotInfo;
	PointLightInfo mPointInfo;
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


namespace managedbindings {
auto GetLightColor(MonoObject* light) -> Vector3;
auto SetLightColor(MonoObject* light, Vector3 color) -> void;

auto GetLightIntensity(MonoObject* light) -> f32;
auto SetLightIntensity(MonoObject* light, f32 intensity) -> void;
}
}
