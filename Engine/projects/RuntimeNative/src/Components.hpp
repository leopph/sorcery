#pragma once

#include "Math.hpp"
#include "Util.hpp"
#include "ManagedAccessObject.hpp"
#include "Material.hpp"

#include <span>
#include <vector>


using MonoReflectionType = struct _MonoReflectionType;


namespace leopph {
	class Entity;
	class Transform;
	class Component;


	class Component : public ManagedAccessObject {
		Entity* mEntity;

	public:
		[[nodiscard]] LEOPPHAPI auto GetEntity() const -> Entity*;
		LEOPPHAPI auto SetEntity(Entity* entity) -> void;
		[[nodiscard]] LEOPPHAPI auto GetTransform() const -> Transform&;

		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;
	};


	namespace managedbindings {
		auto GetComponentEntity(MonoObject* component) -> MonoObject*;
		auto GetComponentEntityTransform(MonoObject* component) -> MonoObject*;
	}


	enum class Space : u8 {
		World = 0,
		Local = 1
	};


	class Transform : public Component {
	private:
		auto UpdateWorldPositionRecursive() -> void;
		auto UpdateWorldRotationRecursive() -> void;
		auto UpdateWorldScaleRecursive() -> void;
		auto UpdateMatrices() -> void;

		Vector3 mLocalPosition{ 0, 0, 0 };
		Quaternion mLocalRotation{ 1, 0, 0, 0 };
		Vector3 mLocalScale{ 1, 1, 1 };

		Vector3 mWorldPosition{ mLocalPosition };
		Quaternion mWorldRotation{ mLocalRotation };
		Vector3 mWorldScale{ mLocalScale };

		Vector3 mForward{ Vector3::forward() };
		Vector3 mRight{ Vector3::right() };
		Vector3 mUp{ Vector3::up() };

		Transform* mParent{ nullptr };
		std::vector<Transform*> mChildren;

		Matrix4 mModelMat{ Matrix4::identity() };
		Matrix3 mNormalMat{ Matrix4::identity() };

	public:
		LEOPPHAPI Transform();

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

		[[nodiscard]] LEOPPHAPI auto GetWorldPosition() const -> Vector3 const&;
		LEOPPHAPI auto SetWorldPosition(Vector3 const& newPos) -> void;

		[[nodiscard]] LEOPPHAPI auto GetLocalPosition() const -> Vector3 const&;
		LEOPPHAPI auto SetLocalPosition(Vector3 const& newPos) -> void;

		[[nodiscard]] LEOPPHAPI auto GetWorldRotation() const -> Quaternion const&;
		LEOPPHAPI auto SetWorldRotation(Quaternion const& newRot) -> void;

		[[nodiscard]] LEOPPHAPI auto GetLocalRotation() const -> Quaternion const&;
		LEOPPHAPI auto SetLocalRotation(Quaternion const& newRot) -> void;

		[[nodiscard]] LEOPPHAPI auto GetWorldScale() const -> Vector3 const&;
		LEOPPHAPI auto SetWorldScale(Vector3 const& newScale) -> void;

		[[nodiscard]] LEOPPHAPI auto GetLocalScale() const -> Vector3 const&;
		LEOPPHAPI auto SetLocalScale(Vector3 const& newScale) -> void;

		LEOPPHAPI auto Translate(Vector3 const& vector, Space base = Space::World) -> void;
		LEOPPHAPI auto Translate(f32 x, f32 y, f32 z, Space base = Space::World) -> void;

		LEOPPHAPI auto Rotate(Quaternion const& rotation, Space base = Space::World) -> void;
		LEOPPHAPI auto Rotate(Vector3 const& axis, f32 angleDegrees, Space base = Space::World) -> void;

		LEOPPHAPI auto Rescale(Vector3 const& scaling, Space base = Space::World) -> void;
		LEOPPHAPI auto Rescale(f32 x, f32 y, f32 z, Space base = Space::World) -> void;

		[[nodiscard]] LEOPPHAPI auto GetRightAxis() const -> Vector3 const&;
		[[nodiscard]] LEOPPHAPI auto GetUpAxis() const -> Vector3 const&;
		[[nodiscard]] LEOPPHAPI auto GetForwardAxis() const -> Vector3 const&;

		[[nodiscard]] LEOPPHAPI auto GetParent() const -> Transform*;
		LEOPPHAPI auto SetParent(Transform* parent) -> void;

		[[nodiscard]] LEOPPHAPI auto GetChildren() const -> std::span<Transform* const>;

		[[nodiscard]] LEOPPHAPI auto GetModelMatrix() const -> Matrix4 const&;
		[[nodiscard]] LEOPPHAPI auto GetNormalMatrix() const -> Matrix3 const&;
	};


	namespace managedbindings {
		auto GetTransformWorldPosition(MonoObject* transform) -> Vector3;
		auto SetTransformWorldPosition(MonoObject* transform, Vector3 newPos) -> void;

		auto GetTransformLocalPosition(MonoObject* transform) -> Vector3;
		auto SetTransformLocalPosition(MonoObject* transform, Vector3 newPos) -> void;

		auto GetTransformWorldRotation(MonoObject* transform) -> Quaternion;
		auto SetTransformWorldRotation(MonoObject* transform, Quaternion newRot) -> void;

		auto GetTransformLocalRotation(MonoObject* transform) -> Quaternion;
		auto SetTransformLocalRotation(MonoObject* transform, Quaternion newRot) -> void;

		auto GetTransformWorldScale(MonoObject* transform) -> Vector3;
		auto SetTransformWorldScale(MonoObject* transform, Vector3 newScale) -> void;

		auto GetTransformLocalScale(MonoObject* transform) -> Vector3;
		auto SetTransformLocalScale(MonoObject* transform, Vector3 newScale) -> void;

		auto TranslateTransformVector(MonoObject* transform, Vector3 vector, Space base) -> void;
		auto TranslateTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) -> void;

		auto RotateTransform(MonoObject* transform, Quaternion rotation, Space base) -> void;
		auto RotateTransformAngleAxis(MonoObject* transform, Vector3 axis, f32 angleDegrees, Space base) -> void;

		auto RescaleTransformVector(MonoObject* transform, Vector3 scaling, Space base) -> void;
		auto RescaleTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) -> void;

		auto GetTransformRightAxis(MonoObject* transform) -> Vector3;
		auto GetTransformUpAxis(MonoObject* transform) -> Vector3;
		auto GetTransformForwardAxis(MonoObject* transform) -> Vector3;

		auto GetTransformParent(MonoObject* transform) -> MonoObject*;
		auto SetTransformParent(MonoObject* transform, MonoObject* parent) -> void;

		auto GetTransformModelMatrix(MonoObject* transform) -> Matrix4;
		auto GetTransformNormalMatrix(MonoObject* transform) -> Matrix3;
	}


	class CubeModel : public Component {
		std::shared_ptr<Material> mMat;

	public:
		LEOPPHAPI CubeModel();
		~CubeModel() override;

		LEOPPHAPI [[nodiscard]] auto GetMaterial() const noexcept -> std::shared_ptr<Material>;
		LEOPPHAPI auto SetMaterial(std::shared_ptr<Material> material) noexcept -> void;

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	};


	class Camera : public Component {
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
		static std::vector<Camera*> sAllInstances;

		f32 mNear{ 0.1f };
		f32 mFar{ 100.f };
		NormalizedViewport mViewport{ { 0, 0 }, { 1, 1 } };
		Extent2D<u32> mWindowExtent;
		f32 mAspect;
		Type mType{ Type::Perspective };
		f32 mOrthoSizeHoriz{ 10 };
		f32 mPerspFovHorizDeg{ 90 };
		Vector4 mBackgroundColor{ 0, 0, 0, 1 };


		[[nodiscard]] auto ConvertPerspectiveFov(f32 fov, bool vert2Horiz) const -> f32;

	public:
		LEOPPHAPI Camera();
		~Camera() override;

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Object::Type override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;

		[[nodiscard]] LEOPPHAPI static auto GetAllInstances() -> std::span<Camera* const>;

		[[nodiscard]] LEOPPHAPI auto GetNearClipPlane() const -> f32;
		LEOPPHAPI auto SetNearClipPlane(f32 nearPlane) -> void;

		[[nodiscard]] LEOPPHAPI auto GetFarClipPlane() const -> f32;
		LEOPPHAPI auto SetFarClipPlane(f32 farPlane) -> void;

		// Viewport extents are normalized between 0 and 1.
		[[nodiscard]] LEOPPHAPI auto GetViewport() const -> NormalizedViewport const&;
		LEOPPHAPI auto SetViewport(NormalizedViewport const& viewport) -> void;

		[[nodiscard]] LEOPPHAPI auto GetWindowExtents() const -> Extent2D<u32>;
		LEOPPHAPI auto SetWindowExtents(Extent2D<u32> const& extent) -> void;

		[[nodiscard]] LEOPPHAPI auto GetAspectRatio() const -> f32;

		[[nodiscard]] LEOPPHAPI auto GetOrthographicSize(Side side = Side::Horizontal) const -> f32;
		LEOPPHAPI auto SetOrthoGraphicSize(f32 size, Side side = Side::Horizontal) -> void;

		[[nodiscard]] LEOPPHAPI auto GetPerspectiveFov(Side side = Side::Horizontal) const -> f32;
		LEOPPHAPI auto SetPerspectiveFov(f32 degrees, Side side = Side::Horizontal) -> void;

		[[nodiscard]] LEOPPHAPI auto GetType() const -> Type;
		LEOPPHAPI auto SetType(Type type) -> void;

		[[nodiscard]] LEOPPHAPI auto GetBackgroundColor() const -> Vector4;
		LEOPPHAPI auto SetBackgroundColor(Vector4 const& color) -> void;
	};


	namespace managedbindings {
		auto GetCameraType(MonoObject* camera) -> Camera::Type;
		auto SetCameraType(MonoObject* camera, Camera::Type type) -> void;

		auto GetCameraPerspectiveFov(MonoObject* camera) -> f32;
		auto SetCameraPerspectiveFov(MonoObject* camera, f32 fov) -> void;

		auto GetCameraOrthographicSize(MonoObject* camera) -> f32;
		auto SetCameraOrthographicSize(MonoObject* camera, f32 size) -> void;

		auto GetCameraNearClipPlane(MonoObject* camera) -> f32;
		auto SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane) -> void;

		auto GetCameraFarClipPlane(MonoObject* camera) -> f32;
		auto SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane) -> void;
	}


	class Behavior : public Component {
	public:
		Behavior() = default;
		explicit Behavior(MonoClass* klass);
		LEOPPHAPI ~Behavior() override;

		LEOPPHAPI auto Init(MonoClass* klass) -> void;

		LEOPPHAPI auto OnGui() -> void override;
		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
		LEOPPHAPI auto SerializeTextual(YAML::Node& node) const -> void override;
		LEOPPHAPI auto DeserializeTextual(YAML::Node const& node) -> void override;
	};

	LEOPPHAPI auto init_behaviors() -> void;
	LEOPPHAPI auto tick_behaviors() -> void;
	LEOPPHAPI auto tack_behaviors() -> void;


	class Light : public Component {
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
		Light() = default;
		Light(Light const& other) = default;
		Light(Light&& other) noexcept = default;

	public:
		~Light() override = default;

	protected:
		auto operator=(Light const& other) -> Light& = default;
		auto operator=(Light&& other) noexcept -> Light& = default;

	private:
		bool mCastsShadow{ false };
		Vector3 mColor{ 1.f };
		f32 mIntensity{ 1.f };
	};


	namespace managedbindings {
		auto GetLightColor(MonoObject* light) -> Vector3;
		auto SetLightColor(MonoObject* light, Vector3 color) -> void;

		auto GetLightIntensity(MonoObject* light) -> f32;
		auto SetLightIntensity(MonoObject* light, f32 intensity) -> void;
	}


	class AttenuatedLight : public Light {
	public:
		[[nodiscard]] LEOPPHAPI auto get_range() const -> f32;
		LEOPPHAPI auto set_range(f32 value) -> void;

	protected:
		AttenuatedLight() = default;
		AttenuatedLight(AttenuatedLight const& other) = default;
		AttenuatedLight(AttenuatedLight&& other) noexcept = default;

	public:
		~AttenuatedLight() override = default;

	protected:
		auto operator=(AttenuatedLight const& other) -> AttenuatedLight& = default;
		auto operator=(AttenuatedLight&& other) noexcept -> AttenuatedLight& = default;

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


	class DirectionalLight final : public Light {
	public:
		[[nodiscard]] LEOPPHAPI auto get_direction() const -> Vector3 const&;

		[[nodiscard]] LEOPPHAPI auto get_shadow_near_plane() const -> f32;
		LEOPPHAPI auto set_shadow_near_plane(f32 newVal) -> void;

		LEOPPHAPI DirectionalLight();
		LEOPPHAPI DirectionalLight(DirectionalLight const& other) = delete;
		LEOPPHAPI DirectionalLight(DirectionalLight&& other) noexcept = delete;

		LEOPPHAPI ~DirectionalLight() override;

		LEOPPHAPI auto operator=(DirectionalLight const& other) -> DirectionalLight&;
		LEOPPHAPI auto operator=(DirectionalLight&& other) noexcept -> DirectionalLight&;

		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

	private:
		f32 mShadowNear{ 50.f };
	};


	class PointLight final : public AttenuatedLight {
	public:
		LEOPPHAPI PointLight();
		LEOPPHAPI PointLight(PointLight const& other);
		LEOPPHAPI PointLight(PointLight&& other) noexcept;

		LEOPPHAPI ~PointLight() override;

		LEOPPHAPI auto operator=(PointLight const& other) -> PointLight&;
		LEOPPHAPI auto operator=(PointLight&& other) noexcept -> PointLight&;
	};


	class SpotLight final : public AttenuatedLight {
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
}
