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
		auto GetComponentEntity(MonoObject* const component) -> MonoObject* {
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetEntity()->GetManagedObject();
		}


		auto GetComponentEntityTransform(MonoObject* const component) -> MonoObject* {
			return static_cast<Component*>(ManagedAccessObject::GetNativePtrFromManagedObject(component))->GetTransform().GetManagedObject();
		}
	}



	auto Transform::UpdateWorldDataRecursive() -> void {
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.Rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;
		mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		mForward = mWorldRotation.Rotate(Vector3::forward());
		mRight = mWorldRotation.Rotate(Vector3::right());
		mUp = mWorldRotation.Rotate(Vector3::up());

		mModelMat[0] = Vector4{ mRight * mWorldScale, 0 };
		mModelMat[1] = Vector4{ mUp * mWorldScale, 0 };
		mModelMat[2] = Vector4{ mForward * mWorldScale, 0 };
		mModelMat[3] = Vector4{ mWorldPosition, 1 };

		mNormalMat[0] = mRight / mWorldScale;
		mNormalMat[1] = mUp / mWorldScale;
		mNormalMat[2] = mForward / mWorldScale;

		for (auto* const child : mChildren) {
			child->UpdateWorldDataRecursive();
		}
	}


	Transform::Transform() {
		CreateManagedObject("leopph", "Transform");
	}

	auto Transform::GetWorldPosition() const -> Vector3 const& {
		return mWorldPosition;
	}


	auto Transform::SetWorldPosition(Vector3 const& newPos) -> void {
		if (mParent != nullptr) {
			SetLocalPosition(mParent->mWorldRotation.conjugate().Rotate(newPos) - mParent->mWorldPosition);
		}
		else {
			SetLocalPosition(newPos);
		}
	}


	auto Transform::GetLocalPosition() const -> Vector3 const& {
		return mLocalPosition;
	}


	auto Transform::SetLocalPosition(Vector3 const& newPos) -> void {
		mLocalPosition = newPos;
		UpdateWorldDataRecursive();
	}


	auto Transform::GetWorldRotation() const -> Quaternion const& {
		return mWorldRotation;
	}


	auto Transform::SetWorldRotation(Quaternion const& newRot) -> void {
		if (mParent != nullptr) {
			SetLocalRotation(mParent->mWorldRotation.conjugate() * newRot);
		}
		else {
			SetLocalRotation(newRot);
		}
	}


	auto Transform::GetLocalRotation() const -> Quaternion const& {
		return mLocalRotation;
	}


	auto Transform::SetLocalRotation(Quaternion const& newRot) -> void {
		mLocalRotation = newRot;
		UpdateWorldDataRecursive();
	}


	auto Transform::GetWorldScale() const -> Vector3 const& {
		return mWorldScale;
	}


	auto Transform::SetWorldScale(Vector3 const& newScale) -> void {
		if (mParent != nullptr) {
			SetLocalScale(newScale / mParent->mWorldScale);
		}
		else {
			SetLocalScale(newScale);
		}
	}


	auto Transform::GetLocalScale() const -> Vector3 const& {
		return mLocalScale;
	}


	auto Transform::SetLocalScale(Vector3 const& newScale) -> void {
		mLocalScale = newScale;
		UpdateWorldDataRecursive();
	}


	auto Transform::Translate(Vector3 const& vector, Space const base) -> void {
		if (base == Space::World) {
			SetWorldPosition(mWorldPosition + vector);
		}
		else if (base == Space::Local) {
			SetLocalPosition(mLocalPosition + mLocalRotation.Rotate(vector));
		}
	}


	auto Transform::Translate(f32 const x, f32 const y, f32 const z, Space const base) -> void {
		Translate(Vector3{ x, y, z }, base);
	}


	auto Transform::Rotate(Quaternion const& rotation, Space const base) -> void {
		if (base == Space::World) {
			SetLocalRotation(rotation * mLocalRotation);
		}
		else if (base == Space::Local) {
			SetLocalRotation(mLocalRotation * rotation);
		}
	}


	auto Transform::Rotate(Vector3 const& axis, f32 const amountDegrees, Space const base) -> void {
		Rotate(Quaternion{ axis, amountDegrees }, base);
	}


	auto Transform::Rescale(Vector3 const& scaling, Space const base) -> void {
		if (base == Space::World) {
			SetWorldScale(mWorldScale * scaling);
		}
		else if (base == Space::Local) {
			SetLocalScale(mLocalScale * scaling);
		}
	}


	auto Transform::Rescale(f32 const x, f32 const y, f32 const z, Space const base) -> void {
		Rescale(Vector3{ x, y, z }, base);
	}


	auto Transform::GetRightAxis() const -> Vector3 const& {
		return mRight;
	}


	auto Transform::GetUpAxis() const -> Vector3 const& {
		return mUp;
	}


	auto Transform::GetForwardAxis() const -> Vector3 const& {
		return mForward;
	}


	auto Transform::GetParent() const -> Transform* {
		return mParent;
	}

	auto Transform::SetParent(Transform* const parent) -> void {
		if (mParent) {
			std::erase(mParent->mChildren, this);
		}

		mParent = parent;

		if (mParent) {
			mParent->mChildren.push_back(this);
		}

		UpdateWorldDataRecursive();
	}


	auto Transform::GetChildren() const -> std::span<Transform* const> {
		return mChildren;
	}

	auto Transform::GetModelMatrix() const -> Matrix4 const& {
		return mModelMat;
	}

	auto Transform::GetNormalMatrix() const -> Matrix3 const& {
		return mNormalMat;
	}


	CubeModel::CubeModel() :
		mMat{ gRenderer.GetDefaultMaterial() } {
		gRenderer.RegisterCubeModel(this);
	}


	CubeModel::~CubeModel() {
		gRenderer.UnregisterCubeModel(this);
	}


	auto CubeModel::GetMaterial() const noexcept -> std::shared_ptr<Material> {
		return mMat;
	}


	auto CubeModel::SetMaterial(std::shared_ptr<Material> material) noexcept -> void {
		if (material) {
			mMat = std::move(material);
		}
	}


	std::vector<Camera*> Camera::sAllInstances;


	auto Camera::ConvertPerspectiveFov(f32 const fov, bool const vert2Horiz) const -> f32 {
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


	auto Camera::GetAllInstances() -> std::span<Camera* const> {
		return sAllInstances;
	}


	auto Camera::GetNearClipPlane() const -> f32 {
		return mNear;
	}


	auto Camera::SetNearClipPlane(f32 const nearPlane) -> void {
		mNear = std::max(nearPlane, 0.03f);
	}


	auto Camera::GetFarClipPlane() const -> f32 {
		return mFar;
	}


	auto Camera::SetFarClipPlane(f32 const farPlane) -> void {
		mFar = std::max(farPlane, mNear + 0.1f);
	}


	auto Camera::GetViewport() const -> NormalizedViewport const& {
		return mViewport;
	}

	auto Camera::SetViewport(NormalizedViewport const& viewport) -> void {
		mViewport.extent.width = std::clamp(viewport.extent.width, 0.f, 1.f);
		mViewport.extent.height = std::clamp(viewport.extent.height, 0.f, 1.f);
		mViewport.position.x = std::clamp(viewport.position.x, 0.f, 1.f);
		mViewport.position.y = std::clamp(viewport.position.y, 0.f, 1.f);
	}


	auto Camera::GetWindowExtents() const -> Extent2D<u32> {
		return mWindowExtent;
	}


	auto Camera::GetAspectRatio() const -> f32 {
		return mAspect;
	}


	auto Camera::GetOrthographicSize(Side side) const -> f32 {
		if (side == Side::Horizontal) {
			return mOrthoSizeHoriz;
		}

		if (side == Side::Vertical) {
			return mOrthoSizeHoriz / mAspect;
		}

		return -1;
	}


	auto Camera::SetOrthoGraphicSize(f32 size, Side side) -> void {
		size = std::max(size, 0.1f);

		if (side == Side::Horizontal) {
			mOrthoSizeHoriz = size;
		}
		else if (side == Side::Vertical) {
			mOrthoSizeHoriz = size * mAspect;
		}
	}


	auto Camera::GetPerspectiveFov(Side const side) const -> f32 {
		if (side == Side::Horizontal) {
			return mPerspFovHorizDeg;
		}

		if (side == Side::Vertical) {
			return ConvertPerspectiveFov(mPerspFovHorizDeg, false);
		}

		return -1;
	}


	auto Camera::SetPerspectiveFov(f32 degrees, Side const side) -> void {
		degrees = std::max(degrees, 5.f);

		if (side == Side::Horizontal) {
			mPerspFovHorizDeg = degrees;
		}
		else if (side == Side::Vertical) {
			mPerspFovHorizDeg = ConvertPerspectiveFov(degrees, true);
		}
	}

	auto Camera::GetType() const -> Camera::Type {
		return mType;
	}


	auto Camera::SetType(Type const type) -> void {
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
		auto GetCameraType(MonoObject* camera) -> Camera::Type {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetType();
		}


		auto SetCameraType(MonoObject* camera, Camera::Type type) -> void {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetType(type);
		}


		auto GetCameraPerspectiveFov(MonoObject* camera) -> f32 {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetPerspectiveFov();
		}


		auto SetCameraPerspectiveFov(MonoObject* camera, f32 fov) -> void {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetPerspectiveFov(fov);
		}


		auto GetCameraOrthographicSize(MonoObject* camera) -> f32 {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetOrthographicSize();
		}


		auto SetCameraOrthographicSize(MonoObject* camera, f32 size) -> void {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetOrthoGraphicSize(size);
		}


		auto GetCameraNearClipPlane(MonoObject* camera) -> f32 {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetNearClipPlane();
		}


		auto SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane) -> void {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetNearClipPlane(nearClipPlane);
		}


		auto GetCameraFarClipPlane(MonoObject* camera) -> f32 {
			return static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetFarClipPlane();
		}


		auto SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane) -> void {
			static_cast<Camera*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetFarClipPlane(farClipPlane);
		}
	}


	namespace {
		std::unordered_map<Behavior*, MonoMethod*> gToInit;
		std::unordered_map<Behavior*, MonoMethod*> gToTick;
		std::unordered_map<Behavior*, MonoMethod*> gToTack;


		auto invoke_method_handle_exception(MonoObject* const obj, MonoMethod* const method) -> void {
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


	auto init_behaviors() -> void {
		for (auto const& [behavior, method] : gToInit) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}

		gToInit.clear();
	}


	auto tick_behaviors() -> void {
		for (auto const& [behavior, method] : gToTick) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	auto tack_behaviors() -> void {
		for (auto const& [behavior, method] : gToTack) {
			invoke_method_handle_exception(behavior->GetManagedObject(), method);
		}
	}


	auto Light::GetColor() const -> Vector3 const& {
		return mColor;
	}


	auto Light::SetColor(Vector3 const& newColor) -> void {
		mColor = newColor;
	}


	auto Light::GetIntensity() const -> f32 {
		return mIntensity;
	}


	auto Light::SetIntensity(f32 const newIntensity) -> void {
		if (newIntensity <= 0) {
			//Logger::get_instance().warning(std::format("Ignoring attempt to set light intensity to {}. This value must be positive.", newIntensity)); TODO
			return;
		}

		mIntensity = newIntensity;
	}


	auto Light::is_casting_shadow() const -> bool {
		return mCastsShadow;
	}


	auto Light::set_casting_shadow(bool const newValue) -> void {
		mCastsShadow = newValue;
	}


	auto AttenuatedLight::get_range() const -> f32 {
		return mRange;
	}


	auto AttenuatedLight::set_range(f32 const value) -> void {
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


	auto DirectionalLight::get_direction() const -> Vector3 const& {
		return GetEntity()->GetTransform().GetForwardAxis();
	}


	auto DirectionalLight::get_shadow_near_plane() const -> f32 {
		return mShadowNear;
	}


	auto DirectionalLight::set_shadow_near_plane(f32 const newVal) -> void {
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


	namespace managedbindings {
		auto GetTransformWorldPosition(MonoObject* const transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldPosition();
		}


		auto SetTransformWorldPosition(MonoObject* transform, Vector3 newPos) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldPosition(newPos);
		}


		auto GetTransformLocalPosition(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalPosition();
		}


		auto SetTransformLocalPosition(MonoObject* transform, Vector3 newPos) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalPosition(newPos);
		}


		auto GetTransformWorldRotation(MonoObject* transform) -> Quaternion {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldRotation();
		}


		auto SetTransformWorldRotation(MonoObject* transform, Quaternion newRot) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldRotation(newRot);
		}


		auto GetTransformLocalRotation(MonoObject* transform) -> Quaternion {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalRotation();
		}


		auto SetTransformLocalRotation(MonoObject* transform, Quaternion newRot) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalRotation(newRot);
		}


		auto GetTransformWorldScale(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetWorldScale();
		}


		auto SetTransformWorldScale(MonoObject* transform, Vector3 newScale) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetWorldScale(newScale);
		}


		auto GetTransformLocalScale(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetLocalScale();
		}


		auto SetTransformLocalScale(MonoObject* transform, Vector3 newScale) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->SetLocalScale(newScale);
		}


		auto TranslateTransformVector(MonoObject* transform, Vector3 vector, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(vector, base);
		}


		auto TranslateTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Translate(x, y, z, base);
		}


		auto RotateTransform(MonoObject* transform, Quaternion rotation, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(rotation, base);
		}


		auto RotateTransformAngleAxis(MonoObject* transform, Vector3 axis, f32 angleDegrees, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rotate(axis, angleDegrees, base);
		}


		auto RescaleTransformVector(MonoObject* transform, Vector3 scaling, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(scaling, base);
		}


		auto RescaleTransform(MonoObject* transform, f32 x, f32 y, f32 z, Space base) -> void {
			ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->Rescale(x, y, z, base);
		}


		auto GetTransformRightAxis(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetRightAxis();
		}


		auto GetTransformUpAxis(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetUpAxis();
		}


		auto GetTransformForwardAxis(MonoObject* transform) -> Vector3 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetForwardAxis();
		}


		auto GetTransformParent(MonoObject* transform) -> MonoObject* {
			auto const parent = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetParent();
			return parent ? parent->GetManagedObject() : nullptr;
		}


		auto SetTransformParent(MonoObject* transform, MonoObject* parent) -> void {
			auto const nativeTransform = ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform);
			auto const nativeParent = parent ? static_cast<Transform*>(ManagedAccessObject::GetNativePtrFromManagedObject(parent)) : nullptr;
			nativeTransform->SetParent(nativeParent);
		}


		auto GetTransformModelMatrix(MonoObject* transform) -> Matrix4 {
			return ManagedAccessObject::GetNativePtrFromManagedObjectAs<Transform*>(transform)->GetModelMatrix();
		}


		auto GetTransformNormalMatrix(MonoObject* transform) -> Matrix3 {
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
