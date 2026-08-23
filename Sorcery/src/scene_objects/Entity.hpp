#pragma once

#include "Component.hpp"
#include "SceneObject.hpp"
#include "TransformComponent.hpp"
#include "../object_ptr.hpp"
#include "../observer_ptr.hpp"
#include "../reflection.hpp"

#include <concepts>
#include <memory>
#include <vector>


namespace sorcery {
class Scene;


class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

public:
  SORCERYAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] SORCERYAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  SORCERYAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  SORCERYAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;

  SORCERYAPI Entity();
  SORCERYAPI Entity(Entity const& other);
  SORCERYAPI Entity(Entity&& other) noexcept;

  SORCERYAPI ~Entity() override;

  auto operator=(Entity const& other) -> void = delete;
  auto operator=(Entity&& other) -> void = delete;

  [[nodiscard]] SORCERYAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] SORCERYAPI auto GetScene() const -> ObserverPtr<Scene const>;

  SORCERYAPI auto AddComponent(std::unique_ptr<Component> component) -> void;
  SORCERYAPI auto RemoveComponent(Component& component) -> std::unique_ptr<Component>;

  template<std::derived_from<Component> T>
  auto GetComponent() const -> T*;

  template<std::derived_from<Component> T>
  auto GetComponents(std::vector<T*>& out) const -> std::vector<T*>&;

  template<std::derived_from<Component> T>
  auto GetComponents() const -> std::vector<T*>;

  [[nodiscard]] SORCERYAPI static auto FindEntityByName(std::string_view name) -> ObjectPtr<Entity>;

private:
  [[nodiscard]] auto GetComponentsForSerialization() const -> std::vector<Component*>;
  auto SetComponentFromDeserialization(std::vector<Component*> components) -> void;

  ObserverPtr<Scene const> scene_{nullptr};
  mutable TransformComponent* transform_{nullptr};
  std::vector<std::unique_ptr<Component>> components_;
};
}


#include "entity.inl"
