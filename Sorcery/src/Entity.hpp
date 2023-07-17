#pragma once

#include "SceneObject.hpp"
#include "Component.hpp"

#include <vector>
#include <concepts>

#include "Reflection.hpp"


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND
  friend class Scene;

  ObserverPtr<Scene> mScene{ nullptr };
  mutable ObserverPtr<TransformComponent> mTransform{ nullptr };
  std::vector<ObserverPtr<Component>> mComponents;

public:
  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

  LEOPPHAPI static Type const SerializationType;
  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

private:
  Entity();

public:
  [[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;

private:
  auto SetScene(Scene& scene) noexcept -> void;

public:
  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  LEOPPHAPI auto AddComponent(Component& component) -> void;
  LEOPPHAPI auto RemoveComponent(Component& component) -> void;


  template<std::derived_from<Component> T>
  auto GetComponent() const -> ObserverPtr<T> {
    for (auto const component : mComponents) {
      if (auto const castPtr = dynamic_cast<ObserverPtr<T>>(component)) {
        return castPtr;
      }
    }
    return nullptr;
  }


  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<ObserverPtr<T>>& outComponents) const -> std::vector<ObserverPtr<T>>& {
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
};
}
