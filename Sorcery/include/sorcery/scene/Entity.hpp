#pragma once

#include <concepts>
#include <memory>
#include <vector>

#include <sorcery/object_ptr.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/reflection.hpp>
#include <sorcery/scene/Component.hpp>
#include <sorcery/scene/SceneObject.hpp>
#include <sorcery/scene/TransformComponent.hpp>


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


#include <sorcery/scene/entity.inl>
