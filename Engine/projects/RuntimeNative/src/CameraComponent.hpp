#pragma once

#include "Component.hpp"

#include "Math.hpp"
#include "Util.hpp"


namespace leopph {
	class CameraComponent : public Component {
	public:
		enum class Type : u8 {
			Perspective = 0,
			Orthographic = 1
		};

		enum class Side : u8 {
			Vertical = 0,
			Horizontal = 1
		};

	private:
		static std::vector<CameraComponent*> sAllInstances;

		f32 mNear{ 0.1f };
		f32 mFar{ 100.f };
		NormalizedViewport mViewport{ { 0, 0 }, { 1, 1 } };
		Extent2D<u32> mWindowExtent;
		f32 mAspect;
		Type mType{ Type::Perspective };
		f32 mOrthoSizeHoriz{ 10 };
		f32 mPerspFovHorizDeg{ 90 };
		Vector4 mBackgroundColor{ 0, 0, 0, 1 };


		[[nodiscard]] auto ConvertPerspectiveFov(f32 fov, bool vert2Horiz) const->f32;

	public:
		LEOPPHAPI CameraComponent();
		~CameraComponent() override;

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const->Object::Type override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

		[[nodiscard]] LEOPPHAPI static auto GetAllInstances() -> std::span<CameraComponent* const>;

		[[nodiscard]] LEOPPHAPI auto GetNearClipPlane() const->f32;
		LEOPPHAPI auto SetNearClipPlane(f32 nearPlane) -> void;

		[[nodiscard]] LEOPPHAPI auto GetFarClipPlane() const->f32;
		LEOPPHAPI auto SetFarClipPlane(f32 farPlane) -> void;

		// Viewport extents are normalized between 0 and 1.
		[[nodiscard]] LEOPPHAPI auto GetViewport() const->NormalizedViewport const&;
		LEOPPHAPI auto SetViewport(NormalizedViewport const& viewport) -> void;

		[[nodiscard]] LEOPPHAPI auto GetWindowExtents() const->Extent2D<u32>;
		LEOPPHAPI auto SetWindowExtents(Extent2D<u32> const& extent) -> void;

		[[nodiscard]] LEOPPHAPI auto GetAspectRatio() const->f32;

		[[nodiscard]] LEOPPHAPI auto GetOrthographicSize(Side side = Side::Horizontal) const->f32;
		LEOPPHAPI auto SetOrthoGraphicSize(f32 size, Side side = Side::Horizontal) -> void;

		[[nodiscard]] LEOPPHAPI auto GetPerspectiveFov(Side side = Side::Horizontal) const->f32;
		LEOPPHAPI auto SetPerspectiveFov(f32 degrees, Side side = Side::Horizontal) -> void;

		[[nodiscard]] LEOPPHAPI auto GetType() const->Type;
		LEOPPHAPI auto SetType(Type type) -> void;

		[[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const->Vector4;
		LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;
	};


	namespace managedbindings {
		auto GetCameraType(MonoObject* camera) -> CameraComponent::Type;
		auto SetCameraType(MonoObject* camera, CameraComponent::Type type) -> void;

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