#include "sorcery/world.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

#include "sorcery/host/rendering/render_world.hpp"
#include "sorcery/scene/Entity.hpp"


namespace sorcery {
auto World::Start() -> void {
  entity_query_memory_.clear();
  std::ranges::transform(entities_, std::back_inserter(entity_query_memory_),
    [](auto const& entity) { return MakeObserver(entity.get()); });

  for (auto const entity : entity_query_memory_) {
    if (entity->IsUpdatable()) {
      entity->Start();
    }

    component_query_memory_.clear();

    for (auto* const component : entity->GetComponents(component_query_memory_)) {
      if (component->IsUpdatable()) {
        component->Start();
      }
    }
  }
}


auto World::Update() -> void {
  entity_query_memory_.clear();
  std::ranges::transform(entities_, std::back_inserter(entity_query_memory_),
    [](auto const& entity) { return MakeObserver(entity.get()); });

  for (auto const& entity : entity_query_memory_) {
    if (entity->IsUpdatable()) {
      entity->Update();
    }

    component_query_memory_.clear();

    for (auto* const component : entity->GetComponents(component_query_memory_)) {
      if (component->IsUpdatable()) {
        component->Update();
      }
    }
  }
}


auto World::GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const> {
  return entities_;
}


auto World::AddEntity(std::unique_ptr<Entity> entity) -> void {
  if (entity) {
    entities_.emplace_back(std::move(entity));
    entities_.back()->OnAfterEnteringWorld(*this);
  }
}


auto World::RemoveEntity(Entity const& entity) -> std::unique_ptr<Entity> {
  if (auto const it{
    std::ranges::find_if(entities_, [&entity](std::unique_ptr<Entity> const& owned_entity) {
      return owned_entity.get() == &entity;
    })
  }; it != std::end(entities_)) {
    (*it)->OnBeforeExitingWorld(*this);
    auto ret{std::move(*it)};
    entities_.erase(it);
    return ret;
  }

  return nullptr;
}


auto World::Clear() -> void {
  for (auto const& entity : entities_) {
    entity->OnBeforeExitingWorld(*this);
  }

  entities_.clear();
}


// Logically const
// ReSharper disable once CppMemberFunctionMayBeConst
auto World::GetRenderWorld() -> rendering::RenderWorld& {
  return *render_world_;
}


auto World::GetRenderWorld() const -> rendering::RenderWorld const& {
  return *render_world_;
}


auto World::GetCameras() const -> std::span<ObserverPtr<CameraComponent> const> {
  return cameras_;
}


auto World::RegisterCamera(CameraComponent& camera) -> void {
  auto const obs = MakeObserver(&camera);
  assert(std::ranges::find(cameras_, obs) == std::ranges::end(cameras_));
  cameras_.emplace_back(obs);
}


auto World::UnregisterCamera(CameraComponent const& camera) -> void {
  std::erase(cameras_, MakeObserver(&camera));
}


World::World() :
  render_world_{std::make_unique<rendering::RenderWorld>()} {}


World::~World() {
  for (auto const& entity : entities_) {
    entity->OnBeforeExitingWorld(*this);
  }
}
}
