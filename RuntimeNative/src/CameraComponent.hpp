#pragma once

#include "Component.hpp"
#include "RenderCamera.hpp"

#include "Math.hpp"
#include "Util.hpp"


namespace leopph {
class CameraComponent : public Component, public RenderCamera {
	f32 mNear{ 0.1f };
	f32 mFar{ 100.f };
	NormalizedViewport mViewport{ { 0, 0 }, { 1, 1 } };
	RenderCamera::Type mType{ RenderCamera::Type::Perspective };
	f32 mOrthoSizeHoriz{ 10 };
	f32 mPerspFovHorizDeg{ 90 };
	Vector4 mBackgroundColor{ 0, 0, 0, 1 };

public:
	LEOPPHAPI CameraComponent();
	~CameraComponent() override;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Object::Type override;
	LEOPPHAPI static Object::Type const SerializationType;

	LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
	LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

	[[nodiscard]] LEOPPHAPI auto GetNearClipPlane() const noexcept -> f32 override;
	LEOPPHAPI auto SetNearClipPlane(f32 nearPlane) -> void;

	[[nodiscard]] LEOPPHAPI auto GetFarClipPlane() const noexcept -> f32 override;
	LEOPPHAPI auto SetFarClipPlane(f32 farPlane) -> void;

	// Viewport extents are normalized between 0 and 1.
	[[nodiscard]] LEOPPHAPI auto GetViewport() const -> NormalizedViewport const&;
	LEOPPHAPI auto SetViewport(NormalizedViewport const& viewport) -> void;

	[[nodiscard]] LEOPPHAPI auto GetHorizontalOrthographicSize() const -> f32 override;
	LEOPPHAPI auto SetHorizontalOrthographicSize(f32 size) -> void;

	[[nodiscard]] LEOPPHAPI auto GetHorizontalPerspectiveFov() const -> f32 override;
	LEOPPHAPI auto SetHorizontalPerspectiveFov(f32 degrees) -> void;

	[[nodiscard]] LEOPPHAPI auto GetType() const -> RenderCamera::Type override;
	LEOPPHAPI auto SetType(RenderCamera::Type type) -> void;

	[[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4;
	LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;

	LEOPPHAPI auto CreateManagedObject() -> void override;

	[[nodiscard]] auto LEOPPHAPI GetPosition() const noexcept -> Vector3 override;
	[[nodiscard]] auto LEOPPHAPI GetForwardAxis() const noexcept -> Vector3 override;
	[[nodiscard]] auto LEOPPHAPI GetFrustum(float aspectRatio) const -> Frustum override;
};


namespace managedbindings {
auto GetCameraType(MonoObject* camera) -> RenderCamera::Type;
auto SetCameraType(MonoObject* camera, RenderCamera::Type type) -> void;

auto GetCameraPerspectiveFov(MonoObject* camera) -> f32;
auto SetCameraPerspectiveFov(MonoObject* camera, f32 fov) -> void;

auto GetCameraOrthographicSize(MonoObject* camera) -> f32;
auto SetCameraOrthographicSize(MonoObject* camera, f32 size) -> void;

auto GetCameraNearClipPlane(MonoObject* camera) -> f32;
auto SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane) -> void;

auto GetCameraFarClipPlane(MonoObject* camera) -> f32;
auto SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane) -> void;
}
}
