#pragma once

#include "SceneObject.hpp"
#include "Component.hpp"
#include "TransformComponent.hpp"

#include <vector>
#include <concepts>

#include "Reflection.hpp"


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

  ObserverPtr<Scene> mScene{ nullptr };
  mutable ObserverPtr<TransformComponent> mTransform{ nullptr };
  std::vector<ObserverPtr<Component>> mComponents;

public:
  LEOPPHAPI Entity();
  Entity(Entity const& other) = delete;
  Entity(Entity&& other) = delete;

  LEOPPHAPI ~Entity() override;

  auto operator=(Entity const& other) -> void = delete;
  auto operator=(Entity&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;

  LEOPPHAPI auto AddComponent(Component& component) -> void;
  LEOPPHAPI auto RemoveComponent(Component& component) -> void;

  template<std::derived_from<Component> T>
  auto GetComponent() const -> ObserverPtr<T>;

  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<ObserverPtr<T>>& outComponents) const -> std::vector<ObserverPtr<T>>&;

  LEOPPHAPI auto OnDrawProperties() -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;
};


template<std::derived_from<Component> T>
auto Entity::GetComponent() const -> ObserverPtr<T> {
  for (auto const component : mComponents) {
    if (auto const castPtr = dynamic_cast<ObserverPtr<T>>(component)) {
      return castPtr;
    }
  }
  return nullptr;
}


template<std::derived_from<Component> T>
auto Entity::GetComponents(std::vector<ObserverPtr<T>>& outComponents) const -> std::vector<ObserverPtr<T>>& {
  if constexpr (std::is_same_v<T, Component>) {
    std::ranges::copy(mComponents, std::back_inserter(outComponents));
  } else {
    for (auto const component : mComponents) {
      if (auto const castPtr{ dynamic_cast<ObserverPtr<T>>(component) }) {
        outComponents.emplace_back(castPtr);
      }
    }
  }
  return outComponents;
}
}
