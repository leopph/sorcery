#include "TransformComponent.hpp"

#include "Serialization.hpp"

#include <format>
#include <iostream>

RTTR_REGISTRATION {
  rttr::registration::class_<leopph::TransformComponent>{ "TransformComponent" }
    .constructor<>();
}


namespace leopph {
auto TransformComponent::UpdateWorldDataRecursive() -> void {
  SetChanged(true);

  mWorldPosition = mParent != nullptr ? mParent->mWorldPosition + mParent->mWorldRotation.Rotate(mLocalPosition) : mLocalPosition;
  mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;
  mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

  mForward = mWorldRotation.Rotate(Vector3::Forward());
  mRight = mWorldRotation.Rotate(Vector3::Right());
  mUp = mWorldRotation.Rotate(Vector3::Up());

  // SRT transformation order

  mModelMat[0] = Vector4{ mRight * mWorldScale[0], 0 };
  mModelMat[1] = Vector4{ mUp * mWorldScale[1], 0 };
  mModelMat[2] = Vector4{ mForward * mWorldScale[2], 0 };
  mModelMat[3] = Vector4{ mWorldPosition, 1 };

  mNormalMat[0] = mRight / mWorldScale[0];
  mNormalMat[1] = mUp / mWorldScale[1];
  mNormalMat[2] = mForward / mWorldScale[2];

  for (auto* const child : mChildren) {
    child->UpdateWorldDataRecursive();
  }
}


auto TransformComponent::GetWorldPosition() const -> Vector3 const& {
  return mWorldPosition;
}


auto TransformComponent::SetWorldPosition(Vector3 const& newPos) -> void {
  if (mParent != nullptr) {
    SetLocalPosition(mParent->mWorldRotation.Conjugate().Rotate(newPos) - mParent->mWorldPosition);
  } else {
    SetLocalPosition(newPos);
  }
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
  UpdateWorldDataRecursive();
}


auto TransformComponent::GetWorldScale() const -> Vector3 const& {
  return mWorldScale;
}


auto TransformComponent::SetWorldScale(Vector3 const& newScale) -> void {
  if (mParent != nullptr) {
    SetLocalScale(newScale / mParent->mWorldScale);
  } else {
    SetLocalScale(newScale);
  }
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
    SetLocalPosition(mLocalPosition + mLocalRotation.Rotate(vector));
  }
}


auto TransformComponent::Translate(f32 const x, f32 const y, f32 const z, Space const base) -> void {
  Translate(Vector3{ x, y, z }, base);
}


auto TransformComponent::Rotate(Quaternion const& rotation, Space const base) -> void {
  if (base == Space::World) {
    SetLocalRotation(rotation * mLocalRotation);
  } else if (base == Space::Local) {
    SetLocalRotation(mLocalRotation * rotation);
  }
}


auto TransformComponent::Rotate(Vector3 const& axis, f32 const amountDegrees, Space const base) -> void {
  Rotate(Quaternion{ axis, amountDegrees }, base);
}


auto TransformComponent::Rescale(Vector3 const& scaling, Space const base) -> void {
  if (base == Space::World) {
    SetWorldScale(mWorldScale * scaling);
  } else if (base == Space::Local) {
    SetLocalScale(mLocalScale * scaling);
  }
}


auto TransformComponent::Rescale(f32 const x, f32 const y, f32 const z, Space const base) -> void {
  Rescale(Vector3{ x, y, z }, base);
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


auto TransformComponent::SetParent(TransformComponent* const parent) -> void {
  if (mParent) {
    std::erase(mParent->mChildren, this);
  }

  mParent = parent;

  if (mParent) {
    mParent->mChildren.push_back(this);
  }

  UpdateWorldDataRecursive();
}


auto TransformComponent::GetChildren() const -> std::span<TransformComponent* const> {
  return mChildren;
}


auto TransformComponent::GetModelMatrix() const -> Matrix4 const& {
  return mModelMat;
}


auto TransformComponent::GetNormalMatrix() const -> Matrix3 const& {
  return mNormalMat;
}


auto TransformComponent::HasChanged() const noexcept -> bool {
  return mChanged;
}


auto TransformComponent::SetChanged(bool const changed) noexcept -> void {
  mChanged = changed;
}


TransformComponent::~TransformComponent() {
  // SetParent(nullptr) on child removes it from this->mChildren so we cannot iterate
  while (!mChildren.empty()) {
    mChildren.back()->SetParent(nullptr);
  }

  SetParent(nullptr);
}


auto CalculateModelMatrixNoScale(TransformComponent const& transform) noexcept -> Matrix4 {
  auto mtx{ transform.GetModelMatrix() };
  auto const scale{ transform.GetWorldScale() };

  for (int i = 0; i < 3; i++) {
    mtx[i] = Vector4{ Vector3{ mtx[i] } / scale, mtx[i][3] };
  }

  return mtx;
}


Object::Type const TransformComponent::SerializationType{ Type::Transform };


auto TransformComponent::GetSerializationType() const -> Type {
  return Type::Transform;
}


auto TransformComponent::Serialize(YAML::Node& node) const -> void {
  Component::Serialize(node);
  node["position"] = GetLocalPosition();
  node["rotation"] = GetLocalRotation();
  node["scale"] = GetLocalScale();
  if (GetParent()) {
    node["parent"] = GetParent()->GetGuid().ToString();
  }
  for (auto const* const child : mChildren) {
    node["children"].push_back(child->GetGuid().ToString());
  }
}


auto TransformComponent::Deserialize(YAML::Node const& node) -> void {
  SetLocalPosition(node["position"].as<Vector3>(GetLocalPosition()));
  SetLocalRotation(node["rotation"].as<Quaternion>(GetLocalRotation()));
  SetLocalScale(node["scale"].as<Vector3>(GetLocalScale()));
  if (node["parent"]) {
    if (!node["parent"].IsScalar()) {
      auto const guidStr{ node["parent"].as<std::string>() };
      auto const parent{ dynamic_cast<TransformComponent*>(FindObjectByGuid(Guid::Parse(guidStr))) };
      if (!parent) {
        std::cerr << "Failed to deserialize parent of Transform " << GetGuid().ToString() << ". Guid " << guidStr << " does not belong to any Transform." << std::endl;
      }
      SetParent(parent);
    }
  }
  if (node["children"]) {
    if (!node["children"].IsSequence()) {
      std::cerr << "Failed to deserialize children of Transform " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      for (auto it{ node["children"].begin() }; it != node["children"].end(); ++it) {
        if (!it->IsScalar()) {
          std::cerr << "Failed to deserialize a child of Transform " << GetGuid().ToString() << ". Invalid data." << std::endl;
        } else {
          auto const guidStr{ it->as<std::string>() };
          auto const child{ dynamic_cast<TransformComponent*>(FindObjectByGuid(Guid::Parse(guidStr))) };
          if (!child) {
            std::cerr << "Failed to deserialize a child of Transform " << GetGuid().ToString() << ". Guid " << guidStr << " does not belong to any Transform." << std::endl;
          } else {
            child->SetParent(this);
          }
        }
      }
    }
  }
}
}
