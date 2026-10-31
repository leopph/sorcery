#include "sorcery/scene_loader.hpp"

#include <algorithm>

#include "sorcery/world.hpp"
#include "sorcery/scene/Entity.hpp"


namespace sorcery {
SceneLoader::SceneLoader(World& world) noexcept :
  world_{&world} {}


auto SceneLoader::Load(Scene const& scene) -> WorldEntityGroup {
  PruneDeadRecords();

  auto entities = scene.Instantiate();
  auto const group = world_->CreateGroup();

  try {
    world_->AddEntities(entities, group);
  } catch (...) {
    world_->DestroyGroup(group);
    throw;
  }

  loaded_.emplace_back(group, AssetRef<Scene>{scene.GetAssetId()});
  return group;
}


auto SceneLoader::Unload(WorldEntityGroup const group) -> void {
  if (world_->IsGroupAlive(group)) {
    world_->DestroyGroup(group);
  }

  std::erase_if(loaded_, [group](auto const& rec) { return rec.group == group; });
  PruneDeadRecords();
}


auto SceneLoader::GetSourceScene(WorldEntityGroup const group) const -> AssetRef<Scene> {
  if (!world_->IsGroupAlive(group)) {
    return nullptr;
  }

  if (auto const it = std::ranges::find(loaded_, group, &LoadedScene::group); it != std::ranges::end(loaded_)) {
    return it->src_asset;
  }

  return nullptr;
}


auto SceneLoader::PruneDeadRecords() -> void {
  std::erase_if(loaded_, [this](auto const& rec) { return !world_->IsGroupAlive(rec.group); });
}
}
