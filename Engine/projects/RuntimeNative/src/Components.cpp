#include "Components.hpp"

#include "Entity.hpp"
#include "ManagedRuntime.hpp"
#include "RenderCore.hpp"

#include <mono/metadata/appdomain.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/class.h>
#include <mono/metadata/image.h>
#include <mono/metadata/reflection.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <concepts>
#include <cstring>


namespace leopph
{
	namespace
	{
		template<std::derived_from<Component> T>
		Component* instantiate(Entity* const entity)
		{
			return new T{ entity };
		}


		std::unordered_map<std::string, std::unordered_map<std::string, std::function<Component* (Entity*)>>> const gComponentInstantiators
		{
			{"leopph",
				{
					{"CubeModel", instantiate<CubeModel>},
					{"Camera", instantiate<Camera>}
				}
			}
		};
	}

	Component::Component(Entity* const entity) :
		entity{ entity }
	{}


	Transform& Component::GetTransform() const
	{
		return *entity->transform;
	}


	namespace managedbindings
	{
		MonoObject* CreateComponent(Entity* const entity, MonoReflectionType* const componentType)
		{
			auto const managedClass = mono_type_get_class(mono_reflection_type_get_type(componentType));

			if (mono_class_is_subclass_of(managedClass, mono_class_from_name(GetManagedImage(), "leopph", "Behavior"), false))
			{
				return nullptr;
			}

			if (!std::strcmp(mono_class_get_namespace(managedClass), "leopph") && !std::strcmp(mono_class_get_name(managedClass), "Transform"))
			{
				return nullptr;
			}

			auto* const className = mono_class_get_name(managedClass);
			auto* const namespaceName = mono_class_get_namespace(managedClass);

			if (auto const nsIt = gComponentInstantiators.find(namespaceName); nsIt != std::end(gComponentInstantiators))
			{
				if (auto const nameIt = nsIt->second.find(className); nameIt != std::end(nsIt->second))
				{
					auto const component = nameIt->second(entity);
					component->CreateManagedObject(managedClass);
					entity->components.emplace_back(component);
					return component->GetManagedObject();
				}
			}

			return nullptr;
		}


		MonoObject* GetComponentEntity(MonoObject* const component)
		{
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->entity->GetManagedObject();
		}


		MonoObject* GetComponentEntityTransform(MonoObject* const component)
		{
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetTransform().GetManagedObject();
		}
	}


	void Transform::UpdateWorldPositionRecursive()
	{
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.Rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;

		UpdateMatrices();

		for (auto* const child : mChildren)
		{
			child->UpdateWorldPositionRecursive();
		}
	}


	void Transform::UpdateWorldRotationRecursive()
	{
		mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;

		mForward = mWorldRotation.Rotate(Vector3::forward());
		mRight = mWorldRotation.Rotate(Vector3::right());
		mUp = mWorldRotation.Rotate(Vector3::up());

		UpdateMatrices();

		for (auto* const child : mChildren)
		{
			child->UpdateWorldRotationRecursive();
		}
	}


	void Transform::UpdateWorldScaleRecursive()
	{
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		UpdateMatrices();

		for (auto* const child : mChildren)
		{
			child->UpdateWorldScaleRecursive();
		}
	}


	void Transform::UpdateMatrices()
	{
		mModelMat[0] = Vector4{ mRight * mWorldScale, 0 };
		mModelMat[1] = Vector4{ mUp * mWorldScale, 0 };
		mModelMat[2] = Vector4{ mForward * mWorldScale, 0 };
		mModelMat[3] = Vector4{ mWorldPosition, 1 };

		mNormalMat[0] = mRight / mWorldScale;
		mNormalMat[1] = mUp / mWorldScale;
		mNormalMat[2] = mForward / mWorldScale;
	}


	Transform::Transform(Entity* const entity) :
		Component{ entity }
	{
		CreateManagedObject("leopph", "Transform");
	}

	Vector3 const& Transform::GetWorldPosition() const
	{
		return mWorldPosition;
	}


	void Transform::SetWorldPosition(Vector3 const& newPos)
	{
		if (mParent != nullptr)
		{
			SetLocalPosition(mParent->mWorldRotation.conjugate().Rotate(newPos) - mParent->mWorldPosition);
		}
		else
		{
			SetLocalPosition(newPos);
		}
	}


	Vector3 const& Transform::GetLocalPosition() const
	{
		return mLocalPosition;
	}


	void Transform::SetLocalPosition(Vector3 const& newPos)
	{
		mLocalPosition = newPos;
		UpdateWorldPositionRecursive();
	}


	Quaternion const& Transform::GetWorldRotation() const
	{
		return mWorldRotation;
	}


	void Transform::SetWorldRotation(Quaternion const& newRot)
	{
		if (mParent != nullptr)
		{
			SetLocalRotation(mParent->mWorldRotation.conjugate() * newRot);
		}
		else
		{
			SetLocalRotation(newRot);
		}
	}


	Quaternion const& Transform::GetLocalRotation() const
	{
		return mLocalRotation;
	}


	void Transform::SetLocalRotation(Quaternion const& newRot)
	{
		mLocalRotation = newRot;
		UpdateWorldRotationRecursive();
	}


	Vector3 const& Transform::GetWorldScale() const
	{
		return mWorldScale;
	}


	void Transform::SetWorldScale(Vector3 const& newScale)
	{
		if (mParent != nullptr)
		{
			SetLocalScale(newScale / mParent->mWorldScale);
		}
		else
		{
			SetLocalScale(newScale);
		}
	}


	Vector3 const& Transform::GetLocalScale() const
	{
		return mLocalScale;
	}


	void Transform::SetLocalScale(Vector3 const& newScale)
	{
		mLocalScale = newScale;
		UpdateWorldScaleRecursive();
	}


	void Transform::Translate(Vector3 const& vector, Space const base)
	{
		if (base == Space::World)
		{
			SetWorldPosition(mWorldPosition + vector);
		}
		else if (base == Space::Local)
		{
			SetLocalPosition(mLocalPosition + mLocalRotation.Rotate(vector));
		}
	}


	void Transform::Translate(f32 const x, f32 const y, f32 const z, Space const base)
	{
		Translate(Vector3{ x, y, z }, base);
	}


	void Transform::Rotate(Quaternion const& rotation, Space const base)
	{
		if (base == Space::World)
		{
			SetLocalRotation(rotation * mLocalRotation);
		}
		else if (base == Space::Local)
		{
			SetLocalRotation(mLocalRotation * rotation);
		}
	}


	void Transform::Rotate(Vector3 const& axis, f32 const amountDegrees, Space const base)
	{
		Rotate(Quaternion{ axis, amountDegrees }, base);
	}


	void Transform::Rescale(Vector3 const& scaling, Space const base)
	{
		if (base == Space::World)
		{
			SetWorldScale(mWorldScale * scaling);
		}
		else if (base == Space::Local)
		{
			SetLocalScale(mLocalScale * scaling);
		}
	}


	void Transform::Rescale(f32 const x, f32 const y, f32 const z, Space const base)
	{
		Rescale(Vector3{ x, y, z }, base);
	}


	Vector3 const& Transform::GetRightAxis() const
	{
		return mRight;
	}


	Vector3 const& Transform::GetUpAxis() const
	{
		return mUp;
	}


	Vector3 const& Transform::GetForwardAxis() const
	{
		return mForward;
	}


	Transform* Transform::GetParent() const
	{
		return mParent;
	}

	void Transform::SetParent(Transform* const parent)
	{
		if (mParent)
		{
			std::erase(mParent->mChildren, this);
		}

		mParent = parent;

		if (mParent)
		{
			mParent->mChildren.push_back(this);
		}

		UpdateWorldPositionRecursive();
		UpdateWorldRotationRecursive();
		UpdateWorldScaleRecursive();
	}


	std::span<Transform* const> Transform::GetChildren() const
	{
		return mChildren;
	}

	Matrix4 const& Transform::GetModelMatrix() const
	{
		return mModelMat;
	}

	Matrix3 const& Transform::GetNormalMatrix() const
	{
		return mNormalMat;
	}


	CubeModel::CubeModel(Entity* const entity) :
		Component{ entity }
	{
		RenderCore::get_last_instance()->register_cube_model(this);
	}


	CubeModel::~CubeModel()
	{
		RenderCore::get_last_instance()->unregister_cube_model(this);
	}


	std::vector<Camera*> Camera::sAllInstances;


	f32 Camera::ConvertPerspectiveFov(f32 const fov, bool const vert2Horiz) const
	{
		if (vert2Horiz)
		{
			return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) * mAspect));
		}

		return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) / mAspect));
	}


	Camera::Camera(Entity* const entity) :
		Component{ entity }
	{
		sAllInstances.emplace_back(this);
	}


	Camera::~Camera()
	{
		std::erase(sAllInstances, this);
	}


	std::span<Camera* const> Camera::GetAllInstances()
	{
		return sAllInstances;
	}

	f32 Camera::GetNearClipPlane() const
	{
		return mNear;
	}

	void Camera::SetNearClipPlane(f32 const nearPlane)
	{
		mNear = nearPlane;
	}

	f32 Camera::GetFarClipPlane() const
	{
		return mFar;
	}

	void Camera::SetFarClipPlane(f32 const farPlane)
	{
		mFar = farPlane;
	}

	NormalizedViewport const& Camera::GetViewport() const
	{
		return mViewport;
	}

	Extent2D<u32> Camera::GetWindowExtents() const
	{
		return mWindowExtent;
	}

	f32 Camera::GetAspectRatio() const
	{
		return mAspect;
	}

	f32 Camera::GetOrthographicSize(Side side) const
	{
		if (side == Side::Horizontal)
		{
			return mOrthoSizeHoriz;
		}

		if (side == Side::Vertical)
		{
			return mOrthoSizeHoriz / mAspect;
		}

		return -1;
	}

	void Camera::SetOrthoGraphicSize(f32 size, Side side)
	{
		if (side == Side::Horizontal)
		{
			mOrthoSizeHoriz = size;
		}
		else if (side == Side::Vertical)
		{
			mOrthoSizeHoriz = size * mAspect;
		}
	}

	f32 Camera::GetPerspectiveFov(Side const side) const
	{
		if (side == Side::Horizontal)
		{
			return mPerspFovHorizDeg;
		}

		if (side == Side::Vertical)
		{
			return ConvertPerspectiveFov(mPerspFovHorizDeg, false);
		}

		return -1;
	}

	void Camera::SetPerspectiveFov(f32 const degrees, Side const side)
	{
		if (side == Side::Horizontal)
		{
			mPerspFovHorizDeg = degrees;
		}
		else if (side == Side::Vertical)
		{
			mPerspFovHorizDeg = ConvertPerspectiveFov(degrees, true);
		}
	}

	Camera::Type Camera::GetType() const
	{
		return mType;
	}

	void Camera::SetType(Type const type)
	{
		mType = type;
	}


	namespace
	{
		std::unordered_map<Behavior*, MonoMethod*> gToInit;
		std::unordered_map<Behavior*, MonoMethod*> gToTick;
		std::unordered_map<Behavior*, MonoMethod*> gToTack;


		void invoke_method_handle_exception(MonoObject* const obj, MonoMethod* const method)
		{
			MonoObject* exception;
			mono_runtime_invoke(method, obj, nullptr, &exception);

			if (exception)
			{
				mono_print_unhandled_exception(exception);
			}
		}
	}


	Behavior::Behavior(Entity* const entity, MonoClass* const klass) :
		Component{ entity }
	{
		auto const obj = CreateManagedObject(klass);

		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0))
		{
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0))
		{
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0))
		{
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0))
		{
			invoke_method_handle_exception(obj, ctor);
		}
	}


	Behavior::~Behavior()
	{
		gToInit.erase(this);
		gToTick.erase(this);
		gToTack.erase(this);

		MonoObject* const managedObj = GetManagedObject();

		if (MonoMethod* const destroyMethod = mono_class_get_method_from_name(mono_object_get_class(managedObj), "OnDestroy", 0))
		{
			invoke_method_handle_exception(managedObj, destroyMethod);
		}
	}


	void init_behaviors()
	{
		for (auto const& [behavior, method] : gToInit)
		{
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}

		gToInit.clear();
	}


	void tick_behaviors()
	{
		for (auto const& [behavior, method] : gToTick)
		{
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	void tack_behaviors()
	{
		for (auto const& [behavior, method] : gToTack)
		{
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	namespace managedbindings
	{
		Vector3 GetTransformWorldPosition(MonoObject* const transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldPosition();
		}


		void SetTransformWorldPosition(MonoObject* transform, Vector3 newPos)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldPosition(newPos);
		}


		Vector3 GetTransformLocalPosition(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalPosition();
		}


		void SetTransformLocalPosition(MonoObject* transform, Vector3 newPos)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalPosition(newPos);
		}


		Quaternion GetTransformWorldRotation(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldRotation();
		}


		void SetTransformWorldRotation(MonoObject* transform, Quaternion newRot)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldRotation(newRot);
		}


		Quaternion GetTransformLocalRotation(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalRotation();
		}


		void SetTransformLocalRotation(MonoObject* transform, Quaternion newRot)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalRotation(newRot);
		}


		Vector3 GetTransformWorldScale(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldScale();
		}


		void SetTransformWorldScale(MonoObject* transform, Vector3 newScale)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldScale(newScale);
		}


		Vector3 GetTransformLocalScale(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalScale();
		}


		void SetTransformLocalScale(MonoObject* transform, Vector3 newScale)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalScale(newScale);
		}


		void TranslateTransformVector(MonoObject* transform, Vector3 vector, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(vector, base);
		}


		void TranslateTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(x, y, z, base);
		}


		void RotateTransform(MonoObject* transform, Quaternion rotation, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(rotation, base);
		}


		void RotateTransformAngleAxis(MonoObject* transform, Vector3 axis, f32 angleDegrees, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(axis, angleDegrees, base);
		}


		void RescaleTransformVector(MonoObject* transform, Vector3 scaling, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(scaling, base);
		}


		void RescaleTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base)
		{
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(x, y, z, base);
		}


		Vector3 GetTransformRightAxis(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetRightAxis();
		}


		Vector3 GetTransformUpAxis(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetUpAxis();
		}


		Vector3 GetTransformForwardAxis(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetForwardAxis();
		}


		MonoObject* GetTransformParent(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetParent()->GetManagedObject();
		}


		void SetTransformParent(MonoObject* transform, MonoObject* parent)
		{
			auto const nativeTransform = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform);
			auto const nativeParent = parent ? static_cast<Transform*>(ManagedAccessObject::GetNativePtrFromManagedObject(parent)) : nullptr;
			nativeTransform->SetParent(nativeParent);
		}


		Matrix4 GetTransformModelMatrix(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetModelMatrix();
		}


		Matrix3 GetTransformNormalMatrix(MonoObject* transform)
		{
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetNormalMatrix();
		}
	}
}