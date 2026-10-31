#pragma once

#include <vector>

#include <sorcery/asset_ref.hpp>
#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>
#include <sorcery/world_entity_group.hpp>
#include <sorcery/assets/Scene.hpp>


namespace sorcery {
class World;


class SceneLoader {
public:
  SORCERYAPI explicit SceneLoader(World& world) noexcept;

  [[nodiscard]] SORCERYAPI
  auto Load(Scene const& scene) -> WorldEntityGroup;
  SORCERYAPI
  auto Unload(WorldEntityGroup group) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetSourceScene(WorldEntityGroup group) const -> AssetRef<Scene>;

private:
  struct LoadedScene {
    WorldEntityGroup group;
    AssetRef<Scene> src_asset;
  };


  auto PruneDeadRecords() -> void;

  ObserverPtr<World> world_;
  std::vector<LoadedScene> loaded_;
};
}
