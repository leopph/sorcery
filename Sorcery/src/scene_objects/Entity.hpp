#pragma once

#include "SceneObject.hpp"
#include "Component.hpp"
#include "TransformComponent.hpp"
#include "../observer_ptr.hpp"
#include "../Reflection.hpp"

#include <concepts>
#include <memory>
#include <vector>


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  LEOPPHAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;

  LEOPPHAPI Entity();
  LEOPPHAPI Entity(Entity const& other);
  LEOPPHAPI Entity(Entity&& other) noexcept;

  LEOPPHAPI ~Entity() override;

  auto operator=(Entity const& other) -> void = delete;
  auto operator=(Entity&& other) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] LEOPPHAPI auto GetScene() const -> ObserverPtr<Scene const>;

  LEOPPHAPI auto AddComponent(std::unique_ptr<Component> component) -> void;
  LEOPPHAPI auto RemoveComponent(Component& component) -> std::unique_ptr<Component>;

  template<std::derived_from<Component> T>
  auto GetComponent() const -> T*;

  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<T*>& out) const -> std::vector<T*>&;

  template<std::derived_from<Component> T>
  auto GetComponents() const -> std::vector<T*>;

  [[nodiscard]] LEOPPHAPI static auto FindEntityByName(std::string_view name) -> Entity*;

private:
  [[nodiscard]] auto GetComponentsForSerialization() const -> std::vector<Component*>;
  auto SetComponentFromDeserialization(std::vector<Component*> components) -> void;

  ObserverPtr<Scene const> scene_{nullptr};
  mutable TransformComponent* transform_{nullptr};
  std::vector<std::unique_ptr<Component>> components_;
};
}


#include "entity.inl"
