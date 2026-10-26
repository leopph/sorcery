#include "sorcery/scene/TransformComponent.hpp"

#include <stdexcept>
#include <utility>

#include <spdlog/spdlog.h>

#include "sorcery/Serialization.hpp"
#include "sorcery/scene/Entity.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::TransformComponent>{"Transform Component"}
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("localPosition", &sorcery::TransformComponent::GetLocalPosition,
      &sorcery::TransformComponent::SetLocalPosition)
    .property("localRotation", &sorcery::TransformComponent::GetLocalRotation,
      &sorcery::TransformComponent::SetLocalRotation)
    .property("localScale", &sorcery::TransformComponent::GetLocalScale, &sorcery::TransformComponent::SetLocalScale)
    .property("parent", &sorcery::TransformComponent::GetParent, &sorcery::TransformComponent::SetParent)
    .property("localEulerHelper", &sorcery::TransformComponent::mLocalEulerAnglesHelp);
}


namespace sorcery {
auto TransformComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<TransformComponent>(*this);
}


TransformComponent::TransformComponent(TransformComponent const& other) :
  Component{other},
  mLocalPosition{other.mLocalPosition},
  mLocalRotation{other.mLocalRotation},
  mLocalEulerAnglesHelp{other.mLocalEulerAnglesHelp},
  mLocalScale{other.mLocalScale} {
  UpdateWorldDataRecursive();
  // Explicitly not copying parent and children
}


TransformComponent::~TransformComponent() {
  // We're doing this in the dtor because we expect transforms to not exist outside an entity.
  // A a transform is created when an entity is created (with no parent, hence no custom ctor),
  // and a transform is destroyed when the entity is destroyed (hence this piece).

  // SetParent(nullptr) on child removes it from this->mChildren so we cannot iterate
  while (!mChildren.empty()) {
    mChildren.back()->SetParent(nullptr);
  }

  SetParent(nullptr);
}


auto TransformComponent::GetWorldPosition() const -> Vector3 const& {
  return mWorldPosition;
}


auto TransformComponent::SetWorldPosition(Vector3 const& newPos) -> void {
  if (!mParent) {
    SetLocalPosition(newPos);
    return;
  }

  auto const scaled_local = mParent->mWorldRotation.Conjugate().Rotate(newPos - mParent->mWorldPosition);
  auto new_local = mLocalPosition;

  for (auto i = 0; i < 3; ++i) {
    if (mParent->mWorldScale[i] != 0.0f) {
      new_local[i] = scaled_local[i] / mParent->mWorldScale[i];
    }
  }

  SetLocalPosition(new_local);
}


auto TransformComponent::GetLocalPosition() const -> Vector3 const& {
  return mLocalPosition;
}


auto TransformComponent::SetLocalPosition(Vector3 const& newPos) -> void {
  mLocalPosition = newPos;
  UpdateWorldDataRecursive();
}


auto TransformComponent::GetWorldRotation() const -> Quaternion const& {
  return mWorldRotation;
}


auto TransformComponent::SetWorldRotation(Quaternion const& newRot) -> void {
  if (mParent != nullptr) {
    SetLocalRotation(mParent->mWorldRotation.Conjugate() * newRot);
  } else {
    SetLocalRotation(newRot);
  }
}


auto TransformComponent::GetLocalRotation() const -> Quaternion const& {
  return mLocalRotation;
}


auto TransformComponent::SetLocalRotation(Quaternion const& newRot) -> void {
  mLocalRotation = newRot;
  mLocalEulerAnglesHelp = newRot.ToEulerAngles();
  UpdateWorldDataRecursive();
}


auto TransformComponent::GetLocalEulerAngles() const noexcept -> Vector3 const& {
  return mLocalEulerAnglesHelp;
}


auto TransformComponent::SetLocalEulerAngles(Vector3 const& eulerAngles) noexcept -> void {
  mLocalRotation = Quaternion::FromEulerAngles(eulerAngles);
  mLocalEulerAnglesHelp = eulerAngles;
  UpdateWorldDataRecursive();
}


auto TransformComponent::GetWorldScale() const -> Vector3 const& {
  return mWorldScale;
}


auto TransformComponent::SetWorldScale(Vector3 const& newScale) -> void {
  if (!mParent) {
    SetLocalScale(newScale);
    return;
  }

  auto new_local = mLocalScale;

  for (auto i = 0; i < 3; ++i) {
    if (mParent->mWorldScale[i] != 0.0f) {
      new_local[i] = newScale[i] / mParent->mWorldScale[i];
    }
  }

  SetLocalScale(new_local);
}


auto TransformComponent::GetLocalScale() const -> Vector3 const& {
  return mLocalScale;
}


auto TransformComponent::SetLocalScale(Vector3 const& newScale) -> void {
  mLocalScale = newScale;
  UpdateWorldDataRecursive();
}


auto TransformComponent::Translate(Vector3 const& vector, Space const base) -> void {
  if (base == Space::World) {
    SetWorldPosition(mWorldPosition + vector);
  } else if (base == Space::Local) {
    SetWorldPosition(mWorldPosition + mWorldRotation.Rotate(vector));
  }
}


auto TransformComponent::Translate(f32 const x, f32 const y, f32 const z, Space const base) -> void {
  Translate(Vector3{x, y, z}, base);
}


auto TransformComponent::Rotate(Quaternion const& rotation, Space const base) -> void {
  if (base == Space::World) {
    SetWorldRotation(rotation * mWorldRotation);
  } else if (base == Space::Local) {
    SetLocalRotation(mLocalRotation * rotation);
  }
}


auto TransformComponent::Rotate(Vector3 const& axis, f32 const amountDegrees, Space const base) -> void {
  Rotate(Quaternion{axis, amountDegrees}, base);
}


auto TransformComponent::Rescale(Vector3 const& scaling, Space const base) -> void {
  if (base == Space::World) {
    SetWorldScale(mWorldScale * scaling);
  } else if (base == Space::Local) {
    SetLocalScale(mLocalScale * scaling);
  }
}


auto TransformComponent::Rescale(f32 const x, f32 const y, f32 const z, Space const base) -> void {
  Rescale(Vector3{x, y, z}, base);
}


auto TransformComponent::GetRightAxis() const -> Vector3 const& {
  return mRight;
}


auto TransformComponent::GetUpAxis() const -> Vector3 const& {
  return mUp;
}


auto TransformComponent::GetForwardAxis() const -> Vector3 const& {
  return mForward;
}


auto TransformComponent::GetParent() const -> TransformComponent* {
  return mParent;
}


auto TransformComponent::SetParent(TransformComponent* parent) -> void {
  auto const world_of = [](TransformComponent const& transform) {
    auto const entity = transform.GetEntity();
    return entity ? entity->GetWorld().Get() : nullptr;
  };

  // Parent-child relationships cannot be cross-world. Either both are in the same world, or they're both in none.
  if (parent && world_of(*parent) != world_of(*this)) {
    auto constexpr msg = "Failed to set transform parent: parent entity is from a different world.";
    spdlog::error(msg);
    throw std::runtime_error{msg};
  }

  if (parent == this) {
    auto constexpr msg = "Failed to set transform parent: tried to set self as parent.";
    spdlog::error(msg);
    throw std::runtime_error{msg};
  }

  // Cycle check
  if (parent) {
    auto p = parent->GetParent();

    while (p) {
      if (p == this) {
        auto constexpr msg = "Failed to set transform parent: cycle detected.";
        spdlog::error(msg);
        throw std::runtime_error{msg};
      }

      p = p->GetParent();
    }
  }

  if (mParent) {
    std::erase(mParent->mChildren, this);
  }

  mParent = parent;

  if (mParent) {
    mParent->mChildren.push_back(this);
  }

  UpdateWorldDataRecursive();
}


auto TransformComponent::GetChildren() const -> std::vector<TransformComponent*> const& {
  return mChildren;
}


auto TransformComponent::GetLocalToWorldMatrix() const noexcept -> Matrix4 const& {
  return mLocalToWorldMtx;
}


auto TransformComponent::CalculateLocalToWorldMatrixWithoutScale() const noexcept -> Matrix4 {
  auto mtx{GetLocalToWorldMatrix()};
  auto const scale{GetWorldScale()};

  for (auto i = 0; i < 3; i++) {
    mtx[i] = Vector4{Vector3{mtx[i]} / scale, mtx[i][3]};
  }

  return mtx;
}


auto TransformComponent::HasChanged() const noexcept -> bool {
  return mChanged;
}


auto TransformComponent::SetChanged(bool const changed) noexcept -> void {
  mChanged = changed;
}


auto TransformComponent::UpdateWorldDataRecursive() -> void {
  SetChanged(true);

  mWorldPosition = mParent != nullptr
                     ? mParent->mWorldPosition + mParent->mWorldRotation.Rotate(mParent->mWorldScale * mLocalPosition)
                     : mLocalPosition;
  mWorldRotation = mParent != nullptr
                     ? mParent->mWorldRotation * mLocalRotation
                     : mLocalRotation;
  mWorldScale = mParent != nullptr
                  ? mParent->mWorldScale * mLocalScale
                  : mLocalScale;

  mForward = mWorldRotation.Rotate(Vector3::Forward());
  mRight = mWorldRotation.Rotate(Vector3::Right());
  mUp = mWorldRotation.Rotate(Vector3::Up());

  // SRT transformation order

  mLocalToWorldMtx[0] = Vector4{mRight * mWorldScale[0], 0};
  mLocalToWorldMtx[1] = Vector4{mUp * mWorldScale[1], 0};
  mLocalToWorldMtx[2] = Vector4{mForward * mWorldScale[2], 0};
  mLocalToWorldMtx[3] = Vector4{mWorldPosition, 1};

  for (auto* const child : mChildren) {
    child->UpdateWorldDataRecursive();
  }
}
}
