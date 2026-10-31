#include "sorcery/world.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <queue>
#include <stdexcept>
#include <unordered_set>

#include <spdlog/spdlog.h>

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


auto World::Instantiate(Scene const& scene) -> WorldEntityGroup {
  auto const group = CreateGroup();
  auto entities = scene.Instantiate();
  AddEntities(entities, group);
  scene_instances_.emplace_back(group, AssetRef<Scene>{scene.GetAssetId()});
  return group;
}


auto World::Unload(WorldEntityGroup const group) -> void {
  assert(
    std::ranges::find(scene_instances_, group, &SceneInstance::group) != std::ranges::end(scene_instances_) &&
    "Scene instance not in this world!");

  DestroyGroup(group);

  std::erase_if(scene_instances_, [group](auto const& inst) {
    return inst.group == group;
  });
}


auto World::CreateGroup() -> WorldEntityGroup {
  WorldEntityGroup const group{next_group_id_++};
  live_groups_.emplace(group);
  return group;
}


auto World::DestroyGroup(WorldEntityGroup const group) -> void {
  assert(IsGroupAlive(group) && "Group is not alive in this world!");

  std::vector<ObserverPtr<Entity const>> entities;

  for (auto const& entity : entities_) {
    if (entity->GetWorldGroup() == group) {
      entities.emplace_back(entity.get());
    }
  }

  RemoveEntities(entities);
  live_groups_.erase(group);
}


auto World::IsGroupAlive(WorldEntityGroup const group) const -> bool {
  return live_groups_.contains(group);
}


auto World::GetEntitiesInGroup(WorldEntityGroup const group) const -> std::vector<ObserverPtr<Entity>> {
  std::vector<ObserverPtr<Entity>> ret;

  if (group.IsNone()) {
    return ret;
  }

  for (auto const& entity : entities_) {
    if (entity->GetWorldGroup() == group) {
      ret.emplace_back(entity.get());
    }
  }

  return ret;
}


auto World::GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const> {
  return entities_;
}


auto World::AddEntities(std::span<std::unique_ptr<Entity>> const entities, WorldEntityGroup const group) -> void {
  assert((group.IsNone() || IsGroupAlive(group)) && "Group must be none or alive in this world!");

  // If any of the entities is null, we ignore the whole set.
  // Strong precondition but safe.
  if (std::ranges::any_of(entities, [](auto const& entity) { return !entity; })) {
    auto constexpr msg = "Failed to add entities to world: entity set contains null!";
    spdlog::error(msg);
    throw std::runtime_error{msg};
  }

  // For each entity in the batch we're adding, we need to validate 2 conditions.
  // Its parent (if any) must be in the batch and its children (if any) must also be in the batch.
  // The case where the entity's parent or child is not in the batch but is in this world violates the precondition,
  // since it means that before the call to this function, there was a parent-child relationship between entities
  // inside and outside this world.

  // Dump all entities in the batch in a lookup table for quick search
  std::unordered_set<ObserverPtr<Entity const>> lookup;
  std::ranges::transform(entities, std::inserter(lookup, lookup.end()),
    [](auto const& entity) { return MakeObserver(entity.get()); });

  for (auto const& entity : entities) {
    assert(!entity->GetWorld() && "Entity cannot be in a world!");
    auto const& transform = entity->GetTransform();

    if (std::ranges::any_of(transform.GetChildren(), [&](auto const* const child_transform) {
      assert(child_transform && "Child transform cannot be null!");
      auto const child_entity = child_transform->GetEntity();
      assert(child_entity && "Child entity cannot be null!");
      assert(!child_entity->GetWorld() && "Child entity cannot be in a world!");
      return !lookup.contains(child_entity);
    })) {
      // The entity has a child outside of this batch. This is an error.
      auto constexpr msg = "Failed to add entities to world: entity set references child outside of this world!";
      spdlog::error(msg);
      throw std::runtime_error{msg};
    }

    if (auto const parent = transform.GetParent()) {
      auto const parent_entity = parent->GetEntity();
      assert(parent_entity && "Parent entity cannot be null!");
      assert(!parent_entity->GetWorld() && "Parent entity cannot be in a world!");

      if (!lookup.contains(parent_entity)) {
        // The entity has a parent outside of this batch. This is an error.
        auto constexpr msg = "Failed to add entities to world: entity set references parent outside of this world!";
        spdlog::error(msg);
        throw std::runtime_error{msg};
      }
    }
  }

  for (auto& entity : entities) {
    entity->SetWorldGroup(group);
    entities_.emplace_back(std::move(entity));
    entities_.back()->OnAfterEnteringWorld(*this);
  }
}


auto World::AddEntity(std::unique_ptr<Entity> entity, WorldEntityGroup const group) -> void {
  AddEntities(std::span{&entity, 1}, group);
}


auto World::RemoveEntities(
  std::span<ObserverPtr<Entity const> const> const entities) -> std::vector<std::unique_ptr<Entity>> {
  if (std::ranges::any_of(entities, [this](auto const entity) { return entity->GetWorld().Get() != this; })) {
    throw std::runtime_error{"Failed to remove entities from world: an entity in the set is not this world!"};
  }

  // Collect all descendants of the entities.
  // Entity subtrees are removed as one unit.

  std::unordered_set<ObserverPtr<Entity const>> entities_to_remove;

  std::queue<ObserverPtr<Entity const>> queue;

  for (auto const entity : entities) {
    queue.emplace(entity);
  }

  while (!queue.empty()) {
    auto const q_entity = queue.front();
    queue.pop();

    // Skip entities reached twice, e.g. when both a parent and its child were passed in.
    if (!entities_to_remove.emplace(q_entity).second) {
      continue;
    }

    for (auto const* const child : q_entity->GetTransform().GetChildren()) {
      assert(child && "Child transform cannot be null!");
      auto const child_entity = child->GetEntity();
      assert(child_entity && "Child entity cannot be null!");
      assert(child_entity->GetWorld().Get() == this && "Child entity must be in this world!");
      queue.emplace(child_entity);
    }
  }

  auto const [first, last] = std::ranges::stable_partition(entities_, [&](auto const& entity_ptr) {
    return !entities_to_remove.contains(MakeObserver(entity_ptr.get()));
  });

  for (auto it = first; it != last; ++it) {
    (*it)->OnBeforeExitingWorld(*this);
    (*it)->SetWorldGroup(WorldEntityGroup{});
  }

  std::vector<std::unique_ptr<Entity>> removed_entities;

  for (auto it = first; it != last; ++it) {
    removed_entities.emplace_back(std::move(*it));
  }

  entities_.erase(first, last);

  // Remove parents pointing into this world.
  for (auto const& entity : removed_entities) {
    auto& transform = entity->GetTransform();

    if (auto const parent = transform.GetParent()) {
      auto const parent_entity = parent->GetEntity();
      assert(parent_entity && "Parent entity cannot be null!");

      if (!entities_to_remove.contains(parent_entity)) {
        transform.SetParent(nullptr);
      }
    }
  }

  return removed_entities;
}


auto World::RemoveEntity(Entity const& entity) -> std::vector<std::unique_ptr<Entity>> {
  auto const obs = MakeObserver(&entity);
  return RemoveEntities(std::span{&obs, 1});
}


auto World::Clear() -> void {
  for (auto const& entity : entities_) {
    entity->OnBeforeExitingWorld(*this);
  }

  entities_.clear();
  scene_instances_.clear();
  live_groups_.clear();
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
