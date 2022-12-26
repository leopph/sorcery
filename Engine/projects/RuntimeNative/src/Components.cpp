#include "Components.hpp"

#include "Entity.hpp"
#include "ManagedRuntime.hpp"
#include "Renderer.hpp"

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
#include <algorithm>
#include <format>

#include "Systems.hpp"


namespace leopph {
	auto Component::GetEntity() const -> Entity* {
		return mEntity;
	}


	auto Component::SetEntity(Entity* const entity) -> void {
		mEntity = entity;
	}


	auto Component::GetTransform() const -> Transform& {
		return mEntity->GetTransform();
	}


	namespace managedbindings {
		MonoObject* GetComponentEntity(MonoObject* const component) {
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetEntity()->GetManagedObject();
		}


		MonoObject* GetComponentEntityTransform(MonoObject* const component) {
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetTransform().GetManagedObject();
		}
	}


	void Transform::UpdateWorldPositionRecursive() {
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.Rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;

		UpdateMatrices();

		for (auto* const child : mChildren) {
			child->UpdateWorldPositionRecursive();
		}
	}


	void Transform::UpdateWorldRotationRecursive() {
		mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;

		mForward = mWorldRotation.Rotate(Vector3::forward());
		mRight = mWorldRotation.Rotate(Vector3::right());
		mUp = mWorldRotation.Rotate(Vector3::up());

		UpdateMatrices();

		for (auto* const child : mChildren) {
			child->UpdateWorldRotationRecursive();
		}
	}


	void Transform::UpdateWorldScaleRecursive() {
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		UpdateMatrices();

		for (auto* const child : mChildren) {
			child->UpdateWorldScaleRecursive();
		}
	}


	void Transform::UpdateMatrices() {
		mModelMat[0] = Vector4{ mRight * mWorldScale, 0 };
		mModelMat[1] = Vector4{ mUp * mWorldScale, 0 };
		mModelMat[2] = Vector4{ mForward * mWorldScale, 0 };
		mModelMat[3] = Vector4{ mWorldPosition, 1 };

		mNormalMat[0] = mRight / mWorldScale;
		mNormalMat[1] = mUp / mWorldScale;
		mNormalMat[2] = mForward / mWorldScale;
	}


	Transform::Transform() {
		CreateManagedObject("leopph", "Transform");
	}

	Vector3 const& Transform::GetWorldPosition() const {
		return mWorldPosition;
	}


	void Transform::SetWorldPosition(Vector3 const& newPos) {
		if (mParent != nullptr) {
			SetLocalPosition(mParent->mWorldRotation.conjugate().Rotate(newPos) - mParent->mWorldPosition);
		}
		else {
			SetLocalPosition(newPos);
		}
	}


	Vector3 const& Transform::GetLocalPosition() const {
		return mLocalPosition;
	}


	void Transform::SetLocalPosition(Vector3 const& newPos) {
		mLocalPosition = newPos;
		UpdateWorldPositionRecursive();
	}


	Quaternion const& Transform::GetWorldRotation() const {
		return mWorldRotation;
	}


	void Transform::SetWorldRotation(Quaternion const& newRot) {
		if (mParent != nullptr) {
			SetLocalRotation(mParent->mWorldRotation.conjugate() * newRot);
		}
		else {
			SetLocalRotation(newRot);
		}
	}


	Quaternion const& Transform::GetLocalRotation() const {
		return mLocalRotation;
	}


	void Transform::SetLocalRotation(Quaternion const& newRot) {
		mLocalRotation = newRot;
		UpdateWorldRotationRecursive();
	}


	Vector3 const& Transform::GetWorldScale() const {
		return mWorldScale;
	}


	void Transform::SetWorldScale(Vector3 const& newScale) {
		if (mParent != nullptr) {
			SetLocalScale(newScale / mParent->mWorldScale);
		}
		else {
			SetLocalScale(newScale);
		}
	}


	Vector3 const& Transform::GetLocalScale() const {
		return mLocalScale;
	}


	void Transform::SetLocalScale(Vector3 const& newScale) {
		mLocalScale = newScale;
		UpdateWorldScaleRecursive();
	}


	void Transform::Translate(Vector3 const& vector, Space const base) {
		if (base == Space::World) {
			SetWorldPosition(mWorldPosition + vector);
		}
		else if (base == Space::Local) {
			SetLocalPosition(mLocalPosition + mLocalRotation.Rotate(vector));
		}
	}


	void Transform::Translate(f32 const x, f32 const y, f32 const z, Space const base) {
		Translate(Vector3{ x, y, z }, base);
	}


	void Transform::Rotate(Quaternion const& rotation, Space const base) {
		if (base == Space::World) {
			SetLocalRotation(rotation * mLocalRotation);
		}
		else if (base == Space::Local) {
			SetLocalRotation(mLocalRotation * rotation);
		}
	}


	void Transform::Rotate(Vector3 const& axis, f32 const amountDegrees, Space const base) {
		Rotate(Quaternion{ axis, amountDegrees }, base);
	}


	void Transform::Rescale(Vector3 const& scaling, Space const base) {
		if (base == Space::World) {
			SetWorldScale(mWorldScale * scaling);
		}
		else if (base == Space::Local) {
			SetLocalScale(mLocalScale * scaling);
		}
	}


	void Transform::Rescale(f32 const x, f32 const y, f32 const z, Space const base) {
		Rescale(Vector3{ x, y, z }, base);
	}


	Vector3 const& Transform::GetRightAxis() const {
		return mRight;
	}


	Vector3 const& Transform::GetUpAxis() const {
		return mUp;
	}


	Vector3 const& Transform::GetForwardAxis() const {
		return mForward;
	}


	Transform* Transform::GetParent() const {
		return mParent;
	}

	void Transform::SetParent(Transform* const parent) {
		if (mParent) {
			std::erase(mParent->mChildren, this);
		}

		mParent = parent;

		if (mParent) {
			mParent->mChildren.push_back(this);
		}

		UpdateWorldPositionRecursive();
		UpdateWorldRotationRecursive();
		UpdateWorldScaleRecursive();
	}


	std::span<Transform* const> Transform::GetChildren() const {
		return mChildren;
	}

	Matrix4 const& Transform::GetModelMatrix() const {
		return mModelMat;
	}

	Matrix3 const& Transform::GetNormalMatrix() const {
		return mNormalMat;
	}


	CubeModel::CubeModel() {
		gRenderer.RegisterCubeModel(this);
	}


	CubeModel::~CubeModel() {
		gRenderer.UnregisterCubeModel(this);
	}


	auto CubeModel::GetMaterial() const noexcept -> Material const& {
		return *mMat;
	}

	auto CubeModel::SetMaterial(Material const& material) noexcept -> void {
		mMat = &material;
	}


	std::vector<Camera*> Camera::sAllInstances;


	f32 Camera::ConvertPerspectiveFov(f32 const fov, bool const vert2Horiz) const {
		if (vert2Horiz) {
			return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) * mAspect));
		}

		return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) / mAspect));
	}


	Camera::Camera() {
		sAllInstances.emplace_back(this);
	}


	Camera::~Camera() {
		std::erase(sAllInstances, this);
	}


	std::span<Camera* const> Camera::GetAllInstances() {
		return sAllInstances;
	}


	f32 Camera::GetNearClipPlane() const {
		return mNear;
	}


	void Camera::SetNearClipPlane(f32 const nearPlane) {
		mNear = std::max(nearPlane, 0.03f);
	}


	f32 Camera::GetFarClipPlane() const {
		return mFar;
	}


	void Camera::SetFarClipPlane(f32 const farPlane) {
		mFar = std::max(farPlane, mNear + 0.1f);
	}


	NormalizedViewport const& Camera::GetViewport() const {
		return mViewport;
	}

	void Camera::SetViewport(NormalizedViewport const& viewport) {
		mViewport.extent.width = std::clamp(viewport.extent.width, 0.f, 1.f);
		mViewport.extent.height = std::clamp(viewport.extent.height, 0.f, 1.f);
		mViewport.position.x = std::clamp(viewport.position.x, 0.f, 1.f);
		mViewport.position.y = std::clamp(viewport.position.y, 0.f, 1.f);
	}


	Extent2D<u32> Camera::GetWindowExtents() const {
		return mWindowExtent;
	}


	f32 Camera::GetAspectRatio() const {
		return mAspect;
	}


	f32 Camera::GetOrthographicSize(Side side) const {
		if (side == Side::Horizontal) {
			return mOrthoSizeHoriz;
		}

		if (side == Side::Vertical) {
			return mOrthoSizeHoriz / mAspect;
		}

		return -1;
	}


	void Camera::SetOrthoGraphicSize(f32 size, Side side) {
		size = std::max(size, 0.1f);

		if (side == Side::Horizontal) {
			mOrthoSizeHoriz = size;
		}
		else if (side == Side::Vertical) {
			mOrthoSizeHoriz = size * mAspect;
		}
	}


	f32 Camera::GetPerspectiveFov(Side const side) const {
		if (side == Side::Horizontal) {
			return mPerspFovHorizDeg;
		}

		if (side == Side::Vertical) {
			return ConvertPerspectiveFov(mPerspFovHorizDeg, false);
		}

		return -1;
	}


	void Camera::SetPerspectiveFov(f32 degrees, Side const side) {
		degrees = std::max(degrees, 5.f);

		if (side == Side::Horizontal) {
			mPerspFovHorizDeg = degrees;
		}
		else if (side == Side::Vertical) {
			mPerspFovHorizDeg = ConvertPerspectiveFov(degrees, true);
		}
	}

	Camera::Type Camera::GetType() const {
		return mType;
	}


	void Camera::SetType(Type const type) {
		mType = type;
	}


	auto Camera::GetBackgroundColor() const -> Vector4 {
		return mBackgroundColor;
	}

	auto Camera::SetBackgroundColor(Vector4 const& color) -> void {
		for (auto i = 0; i < 4; i++) {
			mBackgroundColor[i] = std::clamp(color[i], 0.f, 1.f);
		}
	}


	namespace managedbindings {
		Camera::Type GetCameraType(MonoObject* camera) {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetType();
		}


		void SetCameraType(MonoObject* camera, Camera::Type type) {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetType(type);
		}


		f32 GetCameraPerspectiveFov(MonoObject* camera) {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetPerspectiveFov();
		}


		void SetCameraPerspectiveFov(MonoObject* camera, f32 fov) {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetPerspectiveFov(fov);
		}


		f32 GetCameraOrthographicSize(MonoObject* camera) {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetOrthographicSize();
		}


		void SetCameraOrthographicSize(MonoObject* camera, f32 size) {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetOrthoGraphicSize(size);
		}


		f32 GetCameraNearClipPlane(MonoObject* camera) {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetNearClipPlane();
		}


		void SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane) {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetNearClipPlane(nearClipPlane);
		}


		f32 GetCameraFarClipPlane(MonoObject* camera) {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetFarClipPlane();
		}


		void SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane) {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetFarClipPlane(farClipPlane);
		}
	}


	namespace {
		std::unordered_map<Behavior*, MonoMethod*> gToInit;
		std::unordered_map<Behavior*, MonoMethod*> gToTick;
		std::unordered_map<Behavior*, MonoMethod*> gToTack;


		void invoke_method_handle_exception(MonoObject* const obj, MonoMethod* const method) {
			MonoObject* exception;
			mono_runtime_invoke(method, obj, nullptr, &exception);

			if (exception) {
				mono_print_unhandled_exception(exception);
			}
		}
	}


	Behavior::Behavior(MonoClass* const klass) {
		auto const obj = CreateManagedObject(klass);

		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0)) {
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0)) {
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0)) {
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0)) {
			invoke_method_handle_exception(obj, ctor);
		}
	}


	Behavior::~Behavior() {
		gToInit.erase(this);
		gToTick.erase(this);
		gToTack.erase(this);

		MonoObject* const managedObj = GetManagedObject();

		if (MonoMethod* const destroyMethod = mono_class_get_method_from_name(mono_object_get_class(managedObj), "OnDestroy", 0)) {
			invoke_method_handle_exception(managedObj, destroyMethod);
		}
	}


	auto Behavior::Init(MonoClass* klass) -> void {
		auto const obj = CreateManagedObject(klass);

		if (MonoMethod* const initMethod = mono_class_get_method_from_name(klass, "OnInit", 0)) {
			gToInit[this] = initMethod;
		}

		if (MonoMethod* const tickMethod = mono_class_get_method_from_name(klass, "Tick", 0)) {
			gToTick[this] = tickMethod;
		}

		if (MonoMethod* const tackMethod = mono_class_get_method_from_name(klass, "Tack", 0)) {
			gToTack[this] = tackMethod;
		}

		if (MonoMethod* const ctor = mono_class_get_method_from_name(klass, ".ctor", 0)) {
			invoke_method_handle_exception(obj, ctor);
		}
	}


	void init_behaviors() {
		for (auto const& [behavior, method] : gToInit) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}

		gToInit.clear();
	}


	void tick_behaviors() {
		for (auto const& [behavior, method] : gToTick) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	void tack_behaviors() {
		for (auto const& [behavior, method] : gToTack) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	Vector3 const& Light::GetColor() const {
		return mColor;
	}



	void Light::SetColor(Vector3 const& newColor) {
		mColor = newColor;
	}



	f32 Light::GetIntensity() const {
		return mIntensity;
	}



	void Light::SetIntensity(f32 const newIntensity) {
		if (newIntensity <= 0) {
			//Logger::get_instance().warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", newIntensity)); TODO
			return;
		}

		mIntensity = newIntensity;
	}



	bool Light::is_casting_shadow() const {
		return mCastsShadow;
	}



	void Light::set_casting_shadow(bool const newValue) {
		mCastsShadow = newValue;
	}



	f32 AttenuatedLight::get_range() const {
		return mRange;
	}



	void AttenuatedLight::set_range(f32 const value) {
		mRange = value;
	}



	AmbientLight& AmbientLight::get_instance() {
		static AmbientLight instance;
		return instance;
	}



	Vector3 const& AmbientLight::get_intensity() const {
		return mIntensity;
	}



	void AmbientLight::set_intensity(Vector3 const& intensity) {
		mIntensity = intensity;
	}



	Vector3 const& DirectionalLight::get_direction() const {
		return GetEntity()->GetTransform().GetForwardAxis();
	}



	f32 DirectionalLight::get_shadow_near_plane() const {
		return mShadowNear;
	}



	void DirectionalLight::set_shadow_near_plane(f32 const newVal) {
		mShadowNear = newVal;
	}



	DirectionalLight::DirectionalLight() {
		gRenderer.RegisterDirLight(this);
	}



	DirectionalLight::~DirectionalLight() {
		gRenderer.UnregisterDirLight(this);
	}



	PointLight::PointLight() {
		gRenderer.RegisterPointLight(this);
	}



	PointLight::~PointLight() {
		gRenderer.UnregisterPointLight(this);
	}



	f32 SpotLight::get_inner_angle() const {
		return mInnerAngle;
	}



	void SpotLight::set_inner_angle(f32 const degrees) {
		mInnerAngle = degrees;
	}



	f32 SpotLight::get_outer_angle() const {
		return mOuterAngle;
	}



	void SpotLight::set_outer_angle(f32 const degrees) {
		mOuterAngle = degrees;
	}



	SpotLight::SpotLight() {
		gRenderer.RegisterSpotLight(this);
	}



	SpotLight::~SpotLight() {
		gRenderer.UnregisterSpotLight(this);
	}


	namespace managedbindings {
		Vector3 GetTransformWorldPosition(MonoObject* const transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldPosition();
		}


		void SetTransformWorldPosition(MonoObject* transform, Vector3 newPos) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldPosition(newPos);
		}


		Vector3 GetTransformLocalPosition(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalPosition();
		}


		void SetTransformLocalPosition(MonoObject* transform, Vector3 newPos) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalPosition(newPos);
		}


		Quaternion GetTransformWorldRotation(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldRotation();
		}


		void SetTransformWorldRotation(MonoObject* transform, Quaternion newRot) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldRotation(newRot);
		}


		Quaternion GetTransformLocalRotation(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalRotation();
		}


		void SetTransformLocalRotation(MonoObject* transform, Quaternion newRot) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalRotation(newRot);
		}


		Vector3 GetTransformWorldScale(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldScale();
		}


		void SetTransformWorldScale(MonoObject* transform, Vector3 newScale) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldScale(newScale);
		}


		Vector3 GetTransformLocalScale(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalScale();
		}


		void SetTransformLocalScale(MonoObject* transform, Vector3 newScale) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalScale(newScale);
		}


		void TranslateTransformVector(MonoObject* transform, Vector3 vector, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(vector, base);
		}


		void TranslateTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(x, y, z, base);
		}


		void RotateTransform(MonoObject* transform, Quaternion rotation, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(rotation, base);
		}


		void RotateTransformAngleAxis(MonoObject* transform, Vector3 axis, f32 angleDegrees, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(axis, angleDegrees, base);
		}


		void RescaleTransformVector(MonoObject* transform, Vector3 scaling, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(scaling, base);
		}


		void RescaleTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(x, y, z, base);
		}


		Vector3 GetTransformRightAxis(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetRightAxis();
		}


		Vector3 GetTransformUpAxis(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetUpAxis();
		}


		Vector3 GetTransformForwardAxis(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetForwardAxis();
		}


		MonoObject* GetTransformParent(MonoObject* transform) {
			auto const parent = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetParent();
			return parent ? parent->GetManagedObject() : nullptr;
		}


		void SetTransformParent(MonoObject* transform, MonoObject* parent) {
			auto const nativeTransform = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform);
			auto const nativeParent = parent ? static_cast<Transform*>(ManagedAccessObject::GetNativePtrFromManagedObject(parent)) : nullptr;
			nativeTransform->SetParent(nativeParent);
		}


		Matrix4 GetTransformModelMatrix(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetModelMatrix();
		}


		Matrix3 GetTransformNormalMatrix(MonoObject* transform) {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetNormalMatrix();
		}
	}

	namespace managedbindings {
		auto GetLightColor(MonoObject* light) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Light*>(light)->GetColor();
		}


		auto SetLightColor(MonoObject* light, Vector3 color) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Light*>(light)->SetColor(color);
		}


		auto GetLightIntensity(MonoObject* light) -> f32 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Light*>(light)->GetIntensity();
		}


		auto SetLightIntensity(MonoObject* light, f32 intensity) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Light*>(light)->SetIntensity(intensity);
		}
	}
}
