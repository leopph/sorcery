#include "sorcery/scene/Entity.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "sorcery/util.hpp"
#include "sorcery/world.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/object_registry.hpp"


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


auto Entity::OnAfterEnteringWorld(World& world) -> void {
  world_.Reset(&world);

  auto const entities = world.GetEntities();

  auto const is_name_taken = [this, entities](std::string_view const name) {
    return std::ranges::any_of(entities, [this, name](auto const& entity) {
      return entity.get() != this && entity->GetName() == name;
    });
  };

  // Base name is a name without an " (N)" suffix. Base name of "Foo (2)" is "Foo"
  auto const find_base_name = [](std::string_view const name) {
    std::string_view base_name = name;
    if (base_name.ends_with(')')) {
      if (auto const paren_open = base_name.rfind(" ("); paren_open != std::string_view::npos) {
        auto const digits = base_name.substr(paren_open + 2, base_name.size() - paren_open - 3);

        if (!digits.empty() && std::ranges::all_of(digits, [](char const c) { return c >= '0' && c <= '9'; })) {
          base_name = base_name.substr(0, paren_open);
        }
      }
    }

    return base_name;
  };

  if (is_name_taken(GetName())) {
    // Strip an existing " (N)" suffix so duplicates of "Foo (2)" become "Foo (3)", not "Foo (2) (2)"
    auto base_name = find_base_name(GetName());

    std::string name;

    // If name is already taken, append " (N)" suffix, so a duplicate of "Foo" becomes "Foo (2)".
    // Due to the stripping above, the duplicate of "Foo (2)" will be "Foo (3)"
    for (auto i = 2u; true; ++i) {
      name = std::format("{} ({})", base_name, i);

      if (!is_name_taken(name)) {
        break;
      }
    }

    SetName(name);
  }

  for (auto const& component : components_) {
    component->OnAfterEnteringWorld(world);
  }
}


auto Entity::OnBeforeExitingWorld(World& world) -> void {
  for (auto const& component : components_) {
    component->OnBeforeExitingWorld(world);
  }

  world_.Reset();
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
}


Entity::~Entity() {
  while (!components_.empty()) {
    RemoveComponentUnchecked(*components_.back());
  }
}


auto Entity::GetTransform() const -> TransformComponent& {
  if (!transform_) {
    transform_.Reset(GetComponent<TransformComponent>());
  }
  return *transform_;
}


auto Entity::GetWorld() const -> ObserverPtr<World const> {
  return world_;
}


auto Entity::GetWorldGroup() const -> WorldEntityGroup {
  return world_group_;
}


auto Entity::SetWorldGroup(WorldEntityGroup const group) -> void {
  world_group_ = group;
}


auto Entity::AddComponent(std::unique_ptr<Component> component) -> void {
  if (component) {
    components_.emplace_back(std::move(component));
    components_.back()->entity_ = MakeObserver(this);

    if (world_) {
      components_.back()->OnAfterEnteringWorld(*world_);
    }
  }
}


auto Entity::RemoveComponent(Component& component) -> std::unique_ptr<Component> {
  if (ReflCast<TransformComponent*>(&component)) {
    throw std::invalid_argument{"Cannot remove the TransformComponent of an entity."};
  }

  return RemoveComponentUnchecked(component);
}


auto Entity::RemoveComponentUnchecked(Component& component) -> std::unique_ptr<Component> {
  if (auto const it = std::ranges::find(components_, &component, [](auto const& ptr) { return ptr.get(); });
    it != std::end(components_)) {
    if (world_) {
      (*it)->OnBeforeExitingWorld(*world_);
    }

    (*it)->entity_ = nullptr;

    if (it->get() == transform_.Get()) {
      transform_.Reset();
    }

    auto ret = std::move(*it);
    components_.erase(it);
    return ret;
  }

  return nullptr;
}


auto Entity::FindEntityByName(std::string_view const name) -> ObjectPtr<Entity> {
  static std::vector<ObjectPtr<Entity>> entities;
  App::Instance().GetObjectRegistry().FindObjectsOfType(entities);

  for (auto const entity : entities) {
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
    RemoveComponentUnchecked(*components_.back());
  }

  for (auto* const component : components) {
    AddComponent(std::unique_ptr<Component>{component}); // Taking ownership
  }
}
}
