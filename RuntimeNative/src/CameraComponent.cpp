#include "CameraComponent.hpp"

#include "Serialization.hpp"
#include "Entity.hpp"
#include "Renderer.hpp"
#include "TransformComponent.hpp"

#include <iostream>


RTTR_REGISTRATION {
  rttr::registration::class_<leopph::CameraComponent>{ "CameraComponent" }
    .constructor<>();
}


namespace leopph {
Object::Type const CameraComponent::SerializationType{ Object::Type::Camera };


CameraComponent::CameraComponent() {
  RegisterGameCamera(*this);
}


CameraComponent::~CameraComponent() {
  UnregisterGameCamera(*this);
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


auto CameraComponent::GetBackgroundColor() const -> Vector4 {
  return mBackgroundColor;
}


auto CameraComponent::SetBackgroundColor(Vector4 const& color) -> void {
  for (auto i = 0; i < 4; i++) {
    mBackgroundColor[i] = std::clamp(color[i], 0.f, 1.f);
  }
}


auto CameraComponent::GetPosition() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetWorldPosition();
}


auto CameraComponent::GetRightAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetRightAxis();
}


auto CameraComponent::GetUpAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetUpAxis();
}


auto CameraComponent::GetForwardAxis() const noexcept -> Vector3 {
  return GetEntity()->GetTransform().GetForwardAxis();
}


auto CameraComponent::Serialize(YAML::Node& node) const -> void {
  Component::Serialize(node);
  node["type"] = static_cast<int>(GetType());
  node["fov"] = GetHorizontalPerspectiveFov();
  node["size"] = GetHorizontalOrthographicSize();
  node["near"] = GetNearClipPlane();
  node["far"] = GetFarClipPlane();
  node["background"] = GetBackgroundColor();
}


auto CameraComponent::Deserialize(YAML::Node const& root) -> void {
  if (root["type"]) {
    if (!root["type"].IsScalar()) {
      std::cerr << "Failed to deserialize type of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetType(static_cast<Camera::Type>(root["type"].as<int>(static_cast<int>(GetType()))));
    }
  }
  if (root["fov"]) {
    if (!root["fov"].IsScalar()) {
      std::cerr << "Failed to deserialize field of view of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetHorizontalPerspectiveFov(root["fov"].as<f32>(GetHorizontalPerspectiveFov()));
    }
  }
  if (root["size"]) {
    if (!root["size"].IsScalar()) {
      std::cerr << "Failed to deserialize size of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetHorizontalOrthographicSize(root["size"].as<f32>(GetHorizontalOrthographicSize()));
    }
  }
  if (root["near"]) {
    if (!root["near"].IsScalar()) {
      std::cerr << "Failed to deserialize near clip plane of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetNearClipPlane(root["near"].as<f32>(GetNearClipPlane()));
    }
  }
  if (root["far"]) {
    if (!root["far"].IsScalar()) {
      std::cerr << "Failed to deserialize far clip plane of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetFarClipPlane(root["far"].as<f32>(GetFarClipPlane()));
    }
  }
  if (auto const node{ root["background"] }; node) {
    if (!node.IsSequence()) {
      std::cerr << "Failed to deserialize background color of CameraComponent " << GetGuid().ToString() << ". Invalid data." << std::endl;
    } else {
      SetBackgroundColor(node.as<Vector4>(GetBackgroundColor()));
    }
  }
}


auto CameraComponent::GetSerializationType() const -> Object::Type {
  return Object::Type::Camera;
}
}
