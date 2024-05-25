#pragma once

#include "Component.hpp"
#include "../Math.hpp"


namespace sorcery {
enum class Space : int {
  World = 0,
  Local = 1
};


class TransformComponent : public Component {
  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;

  LEOPPHAPI auto OnAfterAttachedToEntity(Entity& entity) -> void override;
  LEOPPHAPI auto OnBeforeDetachedFromEntity(Entity& entity) -> void override;

  [[nodiscard]] LEOPPHAPI auto GetWorldPosition() const -> Vector3 const&;
  LEOPPHAPI auto SetWorldPosition(Vector3 const& newPos) -> void;

  [[nodiscard]] LEOPPHAPI auto GetLocalPosition() const -> Vector3 const&;
  LEOPPHAPI auto SetLocalPosition(Vector3 const& newPos) -> void;

  [[nodiscard]] LEOPPHAPI auto GetWorldRotation() const -> Quaternion const&;
  LEOPPHAPI auto SetWorldRotation(Quaternion const& newRot) -> void;

  [[nodiscard]] LEOPPHAPI auto GetLocalRotation() const -> Quaternion const&;
  LEOPPHAPI auto SetLocalRotation(Quaternion const& newRot) -> void;

  [[nodiscard]] LEOPPHAPI auto GetLocalEulerAngles() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetLocalEulerAngles(Vector3 const& eulerAngles) noexcept -> void;

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

  [[nodiscard]] LEOPPHAPI auto GetChildren() const -> std::vector<TransformComponent*> const&;

  [[nodiscard]] LEOPPHAPI auto GetLocalToWorldMatrix() const noexcept -> Matrix4 const&;
  [[nodiscard]] LEOPPHAPI auto CalculateLocalToWorldMatrixWithoutScale() const noexcept -> Matrix4;

  [[nodiscard]] LEOPPHAPI auto HasChanged() const noexcept -> bool;
  LEOPPHAPI auto SetChanged(bool changed) noexcept -> void;

private:
  auto UpdateWorldDataRecursive() -> void;

  Vector3 mLocalPosition{0, 0, 0};
  Quaternion mLocalRotation{1, 0, 0, 0};
  Vector3 mLocalEulerAnglesHelp{0, 0, 0};
  Vector3 mLocalScale{1, 1, 1};

  Vector3 mWorldPosition{mLocalPosition};
  Quaternion mWorldRotation{mLocalRotation};
  Vector3 mWorldScale{mLocalScale};

  Vector3 mForward{Vector3::Forward()};
  Vector3 mRight{Vector3::Right()};
  Vector3 mUp{Vector3::Up()};

  TransformComponent* mParent{nullptr};
  std::vector<TransformComponent*> mChildren;

  Matrix4 mLocalToWorldMtx{Matrix4::Identity()};

  bool mChanged{false};
};
}
