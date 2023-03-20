#pragma once

#include <span>

#include "Component.hpp"

#include "Math.hpp"


namespace leopph {
enum class Space : u8 {
	World = 0,
	Local = 1
};


class TransformComponent : public Component {
private:
	auto UpdateWorldDataRecursive() -> void;

	Vector3 mLocalPosition{ 0, 0, 0 };
	Quaternion mLocalRotation{ 1, 0, 0, 0 };
	Vector3 mLocalScale{ 1, 1, 1 };

	Vector3 mWorldPosition{ mLocalPosition };
	Quaternion mWorldRotation{ mLocalRotation };
	Vector3 mWorldScale{ mLocalScale };

	Vector3 mForward{ Vector3::Forward() };
	Vector3 mRight{ Vector3::Right() };
	Vector3 mUp{ Vector3::Up() };

	TransformComponent* mParent{ nullptr };
	std::vector<TransformComponent*> mChildren;

	Matrix4 mModelMat{ Matrix4::Identity() };
	Matrix3 mNormalMat{ Matrix4::Identity() };

	bool mChanged{ false };

public:
	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
	LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

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

	[[nodiscard]] LEOPPHAPI auto GetParent() const -> TransformComponent*;
	LEOPPHAPI auto SetParent(TransformComponent* parent) -> void;

	[[nodiscard]] LEOPPHAPI auto GetChildren() const -> std::span<TransformComponent* const>;

	[[nodiscard]] LEOPPHAPI auto GetModelMatrix() const -> Matrix4 const&;
	[[nodiscard]] LEOPPHAPI auto GetNormalMatrix() const -> Matrix3 const&;

	LEOPPHAPI auto CreateManagedObject() -> void override;

	[[nodiscard]] LEOPPHAPI auto HasChanged() const noexcept -> bool;
	LEOPPHAPI auto SetChanged(bool changed) noexcept -> void;

	LEOPPHAPI ~TransformComponent() override;
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
}
