#pragma once

#include <memory>
#include <span>
#include <unordered_set>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/world_entity_group.hpp>


namespace sorcery {
namespace rendering {
class RenderWorld;
}


class Entity;
class Component;
class CameraComponent;


class World {
public:
  SORCERYAPI
  auto Start() -> void;
  SORCERYAPI
  auto Update() -> void;

  [[nodiscard]] SORCERYAPI
  auto CreateGroup() -> WorldEntityGroup;
  SORCERYAPI
  auto DestroyGroup(WorldEntityGroup group) -> void;
  [[nodiscard]] SORCERYAPI
  auto IsGroupAlive(WorldEntityGroup group) const -> bool;
  [[nodiscard]] SORCERYAPI
  auto GetEntitiesInGroup(WorldEntityGroup group) const -> std::vector<ObserverPtr<Entity>>;

  [[nodiscard]] SORCERYAPI
  auto GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const>;
  SORCERYAPI
  auto AddEntities(std::span<std::unique_ptr<Entity>> entities, WorldEntityGroup group = {}) -> void;
  SORCERYAPI
  auto AddEntity(std::unique_ptr<Entity> entity, WorldEntityGroup group = {}) -> void;
  SORCERYAPI
  auto RemoveEntities(std::span<ObserverPtr<Entity const> const> entities) -> std::vector<std::unique_ptr<Entity>>;
  SORCERYAPI
  auto RemoveEntity(Entity const& entity) -> std::vector<std::unique_ptr<Entity>>;
  SORCERYAPI
  auto Clear() -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRenderWorld() -> rendering::RenderWorld&;
  [[nodiscard]] SORCERYAPI
  auto GetRenderWorld() const -> rendering::RenderWorld const&;

  [[nodiscard]] SORCERYAPI
  auto GetCameras() const -> std::span<ObserverPtr<CameraComponent> const>;
  SORCERYAPI
  auto RegisterCamera(CameraComponent& camera) -> void;
  SORCERYAPI
  auto UnregisterCamera(CameraComponent const& camera) -> void;

  SORCERYAPI World();
  World(World const& other) = delete;
  World(World&& other) noexcept = delete;

  SORCERYAPI
  ~World();

  auto operator=(World const& other) -> World& = delete;
  auto operator=(World&& other) noexcept -> World& = delete;

private:
  std::unique_ptr<rendering::RenderWorld> render_world_;
  std::vector<std::unique_ptr<Entity>> entities_;
  std::vector<ObserverPtr<CameraComponent>> cameras_;
  std::vector<Component*> component_query_memory_;
  std::vector<ObserverPtr<Entity>> entity_query_memory_;
  std::unordered_set<WorldEntityGroup> live_groups_;
  std::uint32_t next_group_id_{1};
};
}
