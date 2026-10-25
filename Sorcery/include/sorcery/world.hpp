#pragma once

#include <memory>
#include <span>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>


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
  auto GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const>;
  SORCERYAPI
  auto AddEntity(std::unique_ptr<Entity> entity) -> void;
  SORCERYAPI
  auto RemoveEntity(Entity const& entity) -> std::unique_ptr<Entity>;
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
};
}
