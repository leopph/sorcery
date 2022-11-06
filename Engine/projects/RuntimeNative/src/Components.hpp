#pragma once

#include "ManagedAccessObject.hpp"
#include "Math.hpp"
#include "Util.hpp"

#include <span>
#include <vector>

using MonoReflectionType = struct _MonoReflectionType;


namespace leopph {
	class Entity;
	class Transform;
	class Component;


	class Component : public ManagedAccessObject {
	public:
		Entity* const entity;

		explicit Component(Entity* entity);

		[[nodiscard]] Transform& GetTransform() const;
	};


	namespace managedbindings {
		MonoObject* GetComponentEntity(MonoObject* component);
		MonoObject* GetComponentEntityTransform(MonoObject* component);
	}


	enum class Space : u8 {
		World = 0,
		Local = 1
	};


	class Transform : public Component {
	private:
		void UpdateWorldPositionRecursive();
		void UpdateWorldRotationRecursive();
		void UpdateWorldScaleRecursive();
		void UpdateMatrices();

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
		Transform(Entity* entity);

		[[nodiscard]] LEOPPHAPI Vector3 const& GetWorldPosition() const;
		LEOPPHAPI void SetWorldPosition(Vector3 const& newPos);

		[[nodiscard]] LEOPPHAPI Vector3 const& GetLocalPosition() const;
		LEOPPHAPI void SetLocalPosition(Vector3 const& newPos);

		[[nodiscard]] LEOPPHAPI Quaternion const& GetWorldRotation() const;
		LEOPPHAPI void SetWorldRotation(Quaternion const& newRot);

		[[nodiscard]] LEOPPHAPI Quaternion const& GetLocalRotation() const;
		LEOPPHAPI void SetLocalRotation(Quaternion const& newRot);

		[[nodiscard]] LEOPPHAPI Vector3 const& GetWorldScale() const;
		LEOPPHAPI void SetWorldScale(Vector3 const& newScale);

		[[nodiscard]] LEOPPHAPI Vector3 const& GetLocalScale() const;
		LEOPPHAPI void SetLocalScale(Vector3 const& newScale);

		LEOPPHAPI void Translate(Vector3 const& vector, Space base = Space::World);
		LEOPPHAPI void Translate(f32 x, f32 y, f32 z, Space base = Space::World);

		LEOPPHAPI void Rotate(Quaternion const& rotation, Space base = Space::World);
		LEOPPHAPI void Rotate(Vector3 const& axis, f32 angleDegrees, Space base = Space::World);

		LEOPPHAPI void Rescale(Vector3 const& scaling, Space base = Space::World);
		LEOPPHAPI void Rescale(f32 x, f32 y, f32 z, Space base = Space::World);

		LEOPPHAPI [[nodiscard]] Vector3 const& GetRightAxis() const;
		LEOPPHAPI [[nodiscard]] Vector3 const& GetUpAxis() const;
		LEOPPHAPI [[nodiscard]] Vector3 const& GetForwardAxis() const;

		LEOPPHAPI [[nodiscard]] Transform* GetParent() const;
		void LEOPPHAPI SetParent(Transform* parent);

		[[nodiscard]] LEOPPHAPI std::span<Transform* const> GetChildren() const;

		LEOPPHAPI [[nodiscard]] Matrix4 const& GetModelMatrix() const;
		LEOPPHAPI [[nodiscard]] Matrix3 const& GetNormalMatrix() const;
	};


	namespace managedbindings {
		Vector3 GetTransformWorldPosition(MonoObject* transform);
		void SetTransformWorldPosition(MonoObject* transform, Vector3 newPos);

		Vector3 GetTransformLocalPosition(MonoObject* transform);
		void SetTransformLocalPosition(MonoObject* transform, Vector3 newPos);

		Quaternion GetTransformWorldRotation(MonoObject* transform);
		void SetTransformWorldRotation(MonoObject* transform, Quaternion newRot);

		Quaternion GetTransformLocalRotation(MonoObject* transform);
		void SetTransformLocalRotation(MonoObject* transform, Quaternion newRot);

		Vector3 GetTransformWorldScale(MonoObject* transform);
		void SetTransformWorldScale(MonoObject* transform, Vector3 newScale);

		Vector3 GetTransformLocalScale(MonoObject* transform);
		void SetTransformLocalScale(MonoObject* transform, Vector3 newScale);

		void TranslateTransformVector(MonoObject* transform, Vector3 vector, Space base);
		void TranslateTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base);

		void RotateTransform(MonoObject* transform, Quaternion rotation, Space base);
		void RotateTransformAngleAxis(MonoObject* transform, Vector3 axis, f32 angleDegrees, Space base);

		void RescaleTransformVector(MonoObject* transform, Vector3 scaling, Space base);
		void RescaleTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base);

		Vector3 GetTransformRightAxis(MonoObject* transform);
		Vector3 GetTransformUpAxis(MonoObject* transform);
		Vector3 GetTransformForwardAxis(MonoObject* transform);

		MonoObject* GetTransformParent(MonoObject* transform);
		void SetTransformParent(MonoObject* transform, MonoObject* parent);

		Matrix4 GetTransformModelMatrix(MonoObject* transform);
		Matrix3 GetTransformNormalMatrix(MonoObject* transform);
	}



	class CubeModel : public Component {
	public:
		CubeModel(Entity* entity);
		~CubeModel() override;
	};


	class Camera : public Component {
	public:
		enum class Type : u8 {
			Perspective = 0, Orthographic = 1
		};

		enum class Side : u8 {
			Vertical = 0, Horizontal = 1
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


		[[nodiscard]] f32 ConvertPerspectiveFov(f32 fov, bool vert2Horiz) const;

	public:
		Camera(Entity* entity);
		~Camera() override;

		LEOPPHAPI [[nodiscard]] static std::span<Camera* const> GetAllInstances();

		LEOPPHAPI [[nodiscard]] f32 GetNearClipPlane() const;
		LEOPPHAPI void SetNearClipPlane(f32 nearPlane);

		LEOPPHAPI [[nodiscard]] f32 GetFarClipPlane() const;
		LEOPPHAPI void SetFarClipPlane(f32 farPlane);

		// Viewport extents are normalized between 0 and 1.
		LEOPPHAPI [[nodiscard]] NormalizedViewport const& GetViewport() const;
		LEOPPHAPI void SetViewport(NormalizedViewport const& viewport);

		LEOPPHAPI [[nodiscard]] Extent2D<u32> GetWindowExtents() const;
		LEOPPHAPI void SetWindowExtents(Extent2D<u32> const& extent);

		LEOPPHAPI [[nodiscard]] f32 GetAspectRatio() const;

		LEOPPHAPI [[nodiscard]] f32 GetOrthographicSize(Side side = Side::Horizontal) const;
		LEOPPHAPI void SetOrthoGraphicSize(f32 size, Side side = Side::Horizontal);

		LEOPPHAPI [[nodiscard]] f32 GetPerspectiveFov(Side side = Side::Horizontal) const;
		LEOPPHAPI void SetPerspectiveFov(f32 degrees, Side side = Side::Horizontal);

		LEOPPHAPI [[nodiscard]] Type GetType() const;
		LEOPPHAPI void SetType(Type type);
	};


	namespace managedbindings {
		Camera::Type GetCameraType(MonoObject* camera);
		void SetCameraType(MonoObject* camera, Camera::Type type);

		f32 GetCameraPerspectiveFov(MonoObject* camera);
		void SetCameraPerspectiveFov(MonoObject* camera, f32 fov);

		f32 GetCameraOrthographicSize(MonoObject* camera);
		void SetCameraOrthographicSize(MonoObject* camera, f32 size);

		f32 GetCameraNearClipPlane(MonoObject* camera);
		void SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane);

		f32 GetCameraFarClipPlane(MonoObject* camera);
		void SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane);
	}


	class Behavior : public Component {
	public:
		Behavior(Entity* entity, MonoClass* klass);
		~Behavior() override;
	};

	LEOPPHAPI void init_behaviors();
	LEOPPHAPI void tick_behaviors();
	LEOPPHAPI void tack_behaviors();
}