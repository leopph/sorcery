#include "Entity.hpp"

#include <format>

#include "TransformComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "CameraComponent.hpp"

#include <functional>
#include <iostream>

#include "SkyboxComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Entity>{ "Entity" };
}


namespace sorcery {
namespace {
std::vector<Entity*> gEntityCache;
}


auto Entity::FindEntityByName(std::string_view const name) -> Entity* {
  FindObjectsOfType(gEntityCache);
  for (auto* const entity : gEntityCache) {
    if (entity->GetName() == name) {
      return entity;
    }
  }
  return nullptr;
}


Entity::Entity() {
  auto constexpr defaultEntityName{ "New Entity" };
  SetName(defaultEntityName);
  FindObjectsOfType(gEntityCache);

  bool isNameUnique{ false };
  std::size_t index{ 1 };
  while (!isNameUnique) {
    isNameUnique = true;
    for (auto const entity : gEntityCache) {
      if (entity != this && entity->GetName() == GetName()) {
        SetName(std::format("{} ({})", defaultEntityName, index));
        ++index;
        isNameUnique = false;
        break;
      }
    }
  }
}


auto Entity::GetScene() const -> Scene& {
  return *mScene;
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!mTransform) {
    mTransform = GetComponent<TransformComponent>();
  }
  return *mTransform;
}


auto Entity::AddComponent(std::shared_ptr<Component> component) -> void {
  if (component) {
    component->SetEntity(this);
    mComponents.push_back(std::move(component));
  }
}


auto Entity::DestroyComponent(Component* const component) -> void {
  if (component) {
    if (component->GetEntity()->GetGuid() != GetGuid() || component->GetEntity()->GetTransform().GetGuid() == component->GetGuid()) {
      return;
    }
    std::erase_if(mComponents, [component](auto const& attachedComponent) {
      return attachedComponent->GetGuid() == component->GetGuid();
    });
  }
}


Object::Type const Entity::SerializationType{ Type::Entity };


auto Entity::SetScene(Scene* const scene) noexcept -> void {
  mScene = scene;
}


auto Entity::GetSerializationType() const -> Type {
  return Type::Entity;
}


auto Entity::Serialize(YAML::Node& node) const -> void {
  node["name"] = GetName().data();
}


auto Entity::Deserialize(YAML::Node const& node) -> void {
  if (!node["name"].IsScalar()) {
    std::cerr << "Failed to deserialize name of Entity " << GetGuid().ToString() << ". Invalid data." << std::endl;
  } else {
    SetName(node["name"].as<std::string>());
  }
}
}
