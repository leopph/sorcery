#include "Entity.hpp"

#include <format>

#include "TransformComponent.hpp"
#include "StaticMeshComponent.hpp"
#include "CameraComponent.hpp"

#include <functional>
#include <iostream>
#include <cassert>

#include "SkyboxComponent.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Entity>{ "Entity" }
    .REFLECT_REGISTER_ENTITY_CTOR
    .property("components", &sorcery::Entity::mComponents)(rttr::policy::prop::as_reference_wrapper);
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


Object::Type const Entity::SerializationType{ Type::Entity };


auto Entity::GetSerializationType() const -> Type {
  return SerializationType;
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
  assert(mScene);
  return *mScene;
}


auto Entity::SetScene(Scene& scene) noexcept -> void {
  mScene = std::addressof(scene);
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!mTransform) {
    mTransform = GetComponent<TransformComponent>();
    assert(mTransform);
  }
  return *mTransform;
}


auto Entity::AddComponent(Component& component) -> void {
  component.SetEntity(*this);
  mComponents.emplace_back(&component);
}


auto Entity::RemoveComponent(Component& component) -> void {
  std::erase_if(mComponents, [&component](auto const storedComponent) {
    return storedComponent == &component;
  });
}
}
