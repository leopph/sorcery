#include "TransformComponent.hpp"

#include "../Serialization.hpp"

#include <imgui.h>
#include <iostream>
#include <utility>

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
auto TransformComponent::OnDrawProperties(bool& changed) -> void {
  Component::OnDrawProperties(changed);

  ImGui::Text("Local Position");
  ImGui::TableNextColumn();

  Vector3 localPos{GetLocalPosition()};
  if (ImGui::DragFloat3("###transformPos", localPos.GetData(), 0.1f)) {
    SetLocalPosition(localPos);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Local Rotation");
  ImGui::TableNextColumn();

  if (auto euler{GetLocalEulerAngles()}; ImGui::DragFloat3("###transformRot", euler.GetData(), 1.0f)) {
    SetLocalEulerAngles(euler);
  }

  ImGui::TableNextColumn();
  ImGui::Text("Local Scale");
  ImGui::TableNextColumn();

  bool static uniformScale{true};
  auto constexpr scaleSpeed{0.01f};

  ImGui::Text("%s", "Uniform");
  ImGui::SameLine();
  ImGui::Checkbox("##UniformScaleCheck", &uniformScale);
  ImGui::SameLine();

  if (uniformScale) {
    f32 scale{GetLocalScale()[0]};
    if (ImGui::DragFloat("###transformScale", &scale, scaleSpeed)) {
      SetLocalScale(Vector3{scale});
    }
  } else {
    Vector3 localScale{GetLocalScale()};
    if (ImGui::DragFloat3("###transformScale", localScale.GetData(), scaleSpeed)) {
      SetLocalScale(localScale);
    }
  }
}


auto TransformComponent::Clone() -> std::unique_ptr<SceneObject> {
  auto clone{Create<TransformComponent>(*this)};
  clone->mChildren.clear();
  clone->mParent = nullptr;
  clone->SetParent(mParent);

  return clone;
}


auto TransformComponent::OnAfterAttachedToEntity(Entity& entity) -> void {
  Component::OnAfterAttachedToEntity(entity);
  UpdateWorldDataRecursive();
}


auto TransformComponent::OnBeforeDetachedFromEntity(Entity& entity) -> void {
  // SetParent(nullptr) on child removes it from this->mChildren so we cannot iterate
  while (!mChildren.empty()) {
    mChildren.back()->SetParent(nullptr);
  }

  SetParent(nullptr);
  Component::OnBeforeDetachedFromEntity(entity);
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


TransformComponent::TransformComponent(TransformComponent&& other) noexcept :
  Component{std::move(other)},
  mLocalPosition{std::move(other.mLocalPosition)},
  mLocalRotation{std::move(other.mLocalRotation)},
  mLocalEulerAnglesHelp{std::move(other.mLocalEulerAnglesHelp)},
  mLocalScale{std::move(other.mLocalScale)} {
  UpdateWorldDataRecursive();
  // Explicitly not copying parent and children
}


auto TransformComponent::UpdateWorldDataRecursive() -> void {
  SetChanged(true);

  mWorldPosition = mParent != nullptr
                     ? mParent->mWorldPosition + mParent->mWorldRotation.Rotate(mLocalPosition)
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
  Translate(Vector3{x, y, z}, base);
}


auto TransformComponent::Rotate(Quaternion const& rotation, Space const base) -> void {
  if (base == Space::World) {
    SetLocalRotation(rotation * mLocalRotation);
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

  for (int i = 0; i < 3; i++) {
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
}
