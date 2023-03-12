#include "CameraComponent.hpp"

#include "Serialization.hpp"

#include <format>
#include <iostream>

namespace leopph {
Object::Type const CameraComponent::SerializationType{ Object::Type::Camera };
std::vector<CameraComponent*> CameraComponent::sAllInstances;


auto CameraComponent::ConvertPerspectiveFov(f32 const fov, bool const vert2Horiz) const -> f32 {
	if (vert2Horiz) {
		return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fov) / 2.0f) * mAspect));
	}

	return ToDegrees(2.0f * std::atan(std::tan(ToRadians(fov) / 2.0f) / mAspect));
}


CameraComponent::CameraComponent() {
	sAllInstances.emplace_back(this);
}


CameraComponent::~CameraComponent() {
	std::erase(sAllInstances, this);
}


auto CameraComponent::GetAllInstances() -> std::span<CameraComponent* const> {
	return sAllInstances;
}


auto CameraComponent::GetNearClipPlane() const -> f32 {
	return mNear;
}


auto CameraComponent::SetNearClipPlane(f32 const nearPlane) -> void {
	mNear = std::max(nearPlane, 0.03f);
}


auto CameraComponent::GetFarClipPlane() const -> f32 {
	return mFar;
}


auto CameraComponent::SetFarClipPlane(f32 const farPlane) -> void {
	mFar = std::max(farPlane, mNear + 0.1f);
}


auto CameraComponent::GetViewport() const -> NormalizedViewport const& {
	return mViewport;
}

auto CameraComponent::SetViewport(NormalizedViewport const& viewport) -> void {
	mViewport.extent.width = std::clamp(viewport.extent.width, 0.f, 1.f);
	mViewport.extent.height = std::clamp(viewport.extent.height, 0.f, 1.f);
	mViewport.position.x = std::clamp(viewport.position.x, 0.f, 1.f);
	mViewport.position.y = std::clamp(viewport.position.y, 0.f, 1.f);
}


auto CameraComponent::GetWindowExtents() const -> Extent2D<u32> {
	return mWindowExtent;
}


auto CameraComponent::GetAspectRatio() const -> f32 {
	return mAspect;
}


auto CameraComponent::GetOrthographicSize(Side side) const -> f32 {
	if (side == Side::Horizontal) {
		return mOrthoSizeHoriz;
	}

	if (side == Side::Vertical) {
		return mOrthoSizeHoriz / mAspect;
	}

	return -1;
}


auto CameraComponent::SetOrthoGraphicSize(f32 size, Side side) -> void {
	size = std::max(size, 0.1f);

	if (side == Side::Horizontal) {
		mOrthoSizeHoriz = size;
	}
	else if (side == Side::Vertical) {
		mOrthoSizeHoriz = size * mAspect;
	}
}


auto CameraComponent::GetPerspectiveFov(Side const side) const -> f32 {
	if (side == Side::Horizontal) {
		return mPerspFovHorizDeg;
	}

	if (side == Side::Vertical) {
		return ConvertPerspectiveFov(mPerspFovHorizDeg, false);
	}

	return -1;
}


auto CameraComponent::SetPerspectiveFov(f32 degrees, Side const side) -> void {
	degrees = std::max(degrees, 5.f);

	if (side == Side::Horizontal) {
		mPerspFovHorizDeg = degrees;
	}
	else if (side == Side::Vertical) {
		mPerspFovHorizDeg = ConvertPerspectiveFov(degrees, true);
	}
}

auto CameraComponent::GetType() const -> CameraComponent::Type {
	return mType;
}


auto CameraComponent::SetType(Type const type) -> void {
	mType = type;
}


auto CameraComponent::GetBackgroundColor() const -> Vector4 {
	return mBackgroundColor;
}

auto CameraComponent::SetBackgroundColor(Vector4 const& color) -> void {
	for (auto i = 0; i < 4; i++) {
		mBackgroundColor[i] = std::clamp(color[i], 0.f, 1.f);
	}
}

auto CameraComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Camera");
}

auto CameraComponent::Serialize(YAML::Node& node) const -> void {
	Component::Serialize(node);
	node["type"] = static_cast<int>(GetType());
	node["fov"] = GetPerspectiveFov();
	node["size"] = GetOrthographicSize();
	node["near"] = GetNearClipPlane();
	node["far"] = GetFarClipPlane();
	node["background"] = GetBackgroundColor();
}


auto CameraComponent::Deserialize(YAML::Node const& root) -> void {
	if (root["type"]) {
		if (!root["type"].IsScalar()) {
			std::cerr << "Failed to deserialize type of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetType(static_cast<Type>(root["type"].as<int>(static_cast<int>(GetType()))));
		}
	}
	if (root["fov"]) {
		if (!root["fov"].IsScalar()) {
			std::cerr << "Failed to deserialize field of view of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetPerspectiveFov(root["fov"].as<leopph::f32>(GetPerspectiveFov()));
		}
	}
	if (root["size"]) {
		if (!root["size"].IsScalar()) {
			std::cerr << "Failed to deserialize size of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetOrthoGraphicSize(root["size"].as<leopph::f32>(GetOrthographicSize()));
		}
	}
	if (root["near"]) {
		if (!root["near"].IsScalar()) {
			std::cerr << "Failed to deserialize near clip plane of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetNearClipPlane(root["near"].as<leopph::f32>(GetNearClipPlane()));
		}
	}
	if (root["far"]) {
		if (!root["far"].IsScalar()) {
			std::cerr << "Failed to deserialize far clip plane of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetFarClipPlane(root["far"].as<leopph::f32>(GetFarClipPlane()));
		}
	}
	if (auto const node{ root["background"] }; node) {
		if (!node.IsSequence()) {
			std::cerr << "Failed to deserialize background color of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
		}
		else {
			SetBackgroundColor(node.as<leopph::Vector4>(GetBackgroundColor()));
		}
	}
}

auto CameraComponent::GetSerializationType() const -> Object::Type {
	return Object::Type::Camera;
}


namespace managedbindings {
auto GetCameraType(MonoObject* camera) -> CameraComponent::Type {
	return static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetType();
}


auto SetCameraType(MonoObject* camera, CameraComponent::Type type) -> void {
	static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetType(type);
}


auto GetCameraPerspectiveFov(MonoObject* camera) -> f32 {
	return static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetPerspectiveFov();
}


auto SetCameraPerspectiveFov(MonoObject* camera, f32 fov) -> void {
	static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetPerspectiveFov(fov);
}


auto GetCameraOrthographicSize(MonoObject* camera) -> f32 {
	return static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetOrthographicSize();
}


auto SetCameraOrthographicSize(MonoObject* camera, f32 size) -> void {
	static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetOrthoGraphicSize(size);
}


auto GetCameraNearClipPlane(MonoObject* camera) -> f32 {
	return static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetNearClipPlane();
}


auto SetCameraNearClipPlane(MonoObject* camera, f32 nearClipPlane) -> void {
	static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetNearClipPlane(nearClipPlane);
}


auto GetCameraFarClipPlane(MonoObject* camera) -> f32 {
	return static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->GetFarClipPlane();
}


auto SetCameraFarClipPlane(MonoObject* camera, f32 farClipPlane) -> void {
	static_cast<CameraComponent*>(ManagedAccessObject::GetNativePtrFromManagedObject(camera))->SetFarClipPlane(farClipPlane);
}
}
}
