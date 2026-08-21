#include "Entity.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <utility>

#include "../Util.hpp"
#include "../Resources/Scene.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Entity>{"Entity"}
    .REFLECT_REGISTER_ENTITY_CTOR
    .property("components", &sorcery::Entity::GetComponentsForSerialization,
      &sorcery::Entity::SetComponentFromDeserialization);
}


namespace sorcery {
auto Entity::OnDrawGizmosSelected() -> void {
  SceneObject::OnDrawGizmosSelected();

  for (auto const& component : components_) {
    component->OnDrawGizmosSelected();
  }
}


auto Entity::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<Entity>(*this);
}


auto Entity::OnAfterEnteringScene(Scene const& scene) -> void {
  scene_.Reset(&scene);

  auto const entities{scene.GetEntities()};

  auto name{GetName()};

  for (std::size_t i{2}; true; i++) {
    auto name_is_unique{true};

    for (auto const& entity : entities) {
      if (entity.get() != this && entity->GetName() == name) {
        name_is_unique = false;
        break;
      }
    }

    if (name_is_unique) {
      break;
    }

    name = std::format("{} ({})", GetName(), i);
  }

  SetName(name);

  for (auto const& component : components_) {
    component->OnAfterEnteringScene(scene);
  }
}


auto Entity::OnBeforeExitingScene(Scene const& scene) -> void {
  for (auto const& component : components_) {
    component->OnBeforeExitingScene(scene);
  }

  scene_.Reset();
}


Entity::Entity() {
  SetName("New Entity");
  AddComponent(std::make_unique<TransformComponent>());
}


Entity::Entity(Entity const& other) :
  SceneObject{other} {
  SetName(other.GetName());

  for (auto const& component : other.components_) {
    AddComponent(static_unique_ptr_cast<Component>(component->Clone()));
  }

  auto const transform{GetComponent<TransformComponent>()};
  transform->SetParent(other.GetComponent<TransformComponent>()->GetParent());
}


Entity::Entity(Entity&& other) noexcept :
  SceneObject{std::move(other)} {
  SetName(other.GetName());

  while (!other.components_.empty()) {
    AddComponent(other.RemoveComponent(*other.components_.back()));
  }

  other.AddComponent(std::make_unique<TransformComponent>());
}


Entity::~Entity() {
  while (!components_.empty()) {
    RemoveComponent(*components_.back());
  }
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!transform_) {
    transform_ = GetComponent<TransformComponent>();
  }
  return *transform_;
}


auto Entity::GetScene() const -> ObserverPtr<Scene const> {
  return scene_;
}


auto Entity::AddComponent(std::unique_ptr<Component> component) -> void {
  if (component) {
    components_.emplace_back(std::move(component));
    components_.back()->OnAfterAttachedToEntity(*this);

    if (scene_) {
      components_.back()->OnAfterEnteringScene(*scene_);
    }
  }
}


auto Entity::RemoveComponent(Component& component) -> std::unique_ptr<Component> {
  if (auto const it{
    std::ranges::find_if(components_, [&component](std::unique_ptr<Component> const& owned_component) {
      return owned_component.get() == std::addressof(component);
    })
  }; it != std::end(components_)) {
    if (scene_) {
      (*it)->OnBeforeExitingScene(*scene_);
    }

    (*it)->OnBeforeDetachedFromEntity(*this);

    auto ret{std::move(*it)};
    components_.erase(it);
    return ret;
  }

  return nullptr;
}


auto Entity::FindEntityByName(std::string_view const name) -> Entity* {
  static std::vector<Entity*> entities;
  FindObjectsOfType(entities);

  for (auto* const entity : entities) {
    if (entity->GetName() == name) {
      return entity;
    }
  }
  return nullptr;
}


auto Entity::GetComponentsForSerialization() const -> std::vector<Component*> {
  std::vector<Component*> ret;
  std::ranges::transform(components_, std::back_inserter(ret), [](auto const& component) {
    return component.get();
  });
  return ret;
}


auto Entity::SetComponentFromDeserialization(std::vector<Component*> components) -> void {
  while (!components_.empty()) {
    RemoveComponent(*components_.back());
  }

  for (auto* const component : components) {
    AddComponent(std::unique_ptr<Component>{component}); // Taking ownership
  }
}
}
