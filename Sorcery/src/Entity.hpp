#pragma once

#include "SceneElement.hpp"
#include "Component.hpp"

#include <memory>
#include <vector>
#include <concepts>

#include "Scene.hpp"


namespace sorcery {
class Scene;


class Entity final : public SceneElement {
  RTTR_ENABLE(SceneElement)
  friend class Scene;

  Scene* mScene{ nullptr };
  mutable TransformComponent* mTransform{ nullptr };
  std::vector<std::shared_ptr<Component>> mComponents;

  Entity();

  auto SetScene(Scene* scene) -> void;

public:
  LEOPPHAPI static auto New() -> Entity*;

  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
  LEOPPHAPI static Type const SerializationType;

  LEOPPHAPI auto Serialize(YAML::Node& node) const -> void override;
  LEOPPHAPI auto Deserialize(YAML::Node const& node) -> void override;

  [[nodiscard]] LEOPPHAPI auto GetScene() const -> Scene&;
  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  LEOPPHAPI auto AddComponent(std::shared_ptr<Component> component) -> void;
  LEOPPHAPI auto DestroyComponent(Component* component) -> void;


  template<std::derived_from<Component> T>
  auto GetComponent() const -> T* {
    for (auto const& component : mComponents) {
      if (auto const castPtr = dynamic_cast<T*>(component.get())) {
        return castPtr;
      }
    }
    return nullptr;
  }


  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<T*>& outComponents) const -> std::vector<T*>& {
    outComponents.clear();
    for (auto const& component : mComponents) {
      if (auto const castPtr = dynamic_cast<T*>(component.get())) {
        outComponents.emplace_back(castPtr);
      }
    }
    return outComponents;
  }
};
}
