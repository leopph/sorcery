#pragma once

#include <concepts>
#include <memory>
#include <vector>

#include <sorcery/object_ptr.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/reflection.hpp>
#include <sorcery/scene_instance.hpp>
#include <sorcery/scene/Component.hpp>
#include <sorcery/scene/SceneObject.hpp>
#include <sorcery/scene/TransformComponent.hpp>


namespace sorcery {
class Entity final : public SceneObject {
  RTTR_ENABLE(SceneObject)
  RTTR_REGISTRATION_FRIEND

public:
  SORCERYAPI auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] SORCERYAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  SORCERYAPI auto OnAfterEnteringWorld(World& world) -> void override;
  SORCERYAPI auto OnBeforeExitingWorld(World& world) -> void override;

  SORCERYAPI Entity();
  SORCERYAPI Entity(Entity const& other);
  Entity(Entity&& other) noexcept = delete;

  SORCERYAPI ~Entity() override;

  auto operator=(Entity const& other) -> void = delete;
  auto operator=(Entity&& other) -> void = delete;

  [[nodiscard]] SORCERYAPI auto GetTransform() const -> TransformComponent&;

  [[nodiscard]] SORCERYAPI auto GetWorld() const -> ObserverPtr<World const>;

  [[nodiscard]] SORCERYAPI
  auto GetSceneInstance() const -> SceneInstanceId;
  SORCERYAPI
  auto SetSceneInstance(SceneInstanceId instance) -> void;

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

  ObserverPtr<World> world_{nullptr};
  mutable TransformComponent* transform_{nullptr};
  std::vector<std::unique_ptr<Component>> components_;
  SceneInstanceId scene_instance_{};
};
}


#include <sorcery/scene/entity.inl>
