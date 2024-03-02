#pragma once

#include "SceneObject.hpp"
#include "Component.hpp"
#include "TransformComponent.hpp"
#include "../Reflection.hpp"

#include <vector>
#include <concepts>


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

  ObserverPtr<Scene> mScene{nullptr};
  mutable ObserverPtr<TransformComponent> mTransform{nullptr};
  std::vector<ObserverPtr<Component>> mComponents;

public:
  LEOPPHAPI Entity();

  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;

  LEOPPHAPI auto AddComponent(Component& component) -> void;
  LEOPPHAPI auto RemoveComponent(Component& component) -> void;

  template<std::derived_from<Component> T>
  auto GetComponent() const -> ObserverPtr<T>;

  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<ObserverPtr<T>>& out) const -> std::vector<ObserverPtr<T>>&;

  template<std::derived_from<Component> T>
  auto GetComponents() const -> std::vector<ObserverPtr<T>>;

  LEOPPHAPI auto OnInit() -> void override;
  LEOPPHAPI auto OnDestroy() -> void override;
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;
};


template<std::derived_from<Component> T>
auto Entity::GetComponent() const -> ObserverPtr<T> {
  if constexpr (std::same_as<Component, T>) {
    return mComponents.empty()
             ? nullptr
             : mComponents.front();
  } else {
    for (auto const component : mComponents) {
      if (auto const castPtr{rttr::rttr_cast<ObserverPtr<T>>(component)}) {
        return castPtr;
      }
    }

    return nullptr;
  }
}


template<std::derived_from<Component> T>
auto Entity::GetComponents(std::vector<ObserverPtr<T>>& out) const -> std::vector<ObserverPtr<T>>& {
  if constexpr (std::is_same_v<T, Component>) {
    out = mComponents;
  } else {
    out.clear();

    for (auto const component : mComponents) {
      if (auto const castPtr{rttr::rttr_cast<ObserverPtr<T>>(component)}) {
        out.emplace_back(castPtr);
      }
    }
  }

  return out;
}


template<std::derived_from<Component> T>
auto Entity::GetComponents() const -> std::vector<ObserverPtr<T>> {
  std::vector<ObserverPtr<T>> ret;
  GetComponents(ret);
  return ret;
}
}
