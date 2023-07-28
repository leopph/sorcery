#pragma once

#include "NativeResource.hpp"
#include "../Entity.hpp"
#include "../Component.hpp"

#include <vector>
#include <span>


namespace sorcery {
class Scene final : public NativeResource {
  RTTR_ENABLE(NativeResource)
  static Scene* sActiveScene;
  static std::vector<Scene*> sAllScenes;


  std::vector<ObserverPtr<Entity>> mEntities;

  YAML::Node mYamlData;

public:
  [[nodiscard]] LEOPPHAPI static auto GetActiveScene() noexcept -> Scene*;

  LEOPPHAPI Scene();
  Scene(Scene const& other) = delete;
  Scene(Scene&& other) = delete;

  LEOPPHAPI ~Scene() override;

  auto operator=(Scene const& other) -> void = delete;
  auto operator=(Scene&& other) -> void = delete;

  LEOPPHAPI auto AddEntity(Entity& entity) -> void;
  LEOPPHAPI auto RemoveEntity(Entity const& entity) -> void;
  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<ObserverPtr<Entity> const>;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load() -> void;

  LEOPPHAPI auto Clear() -> void;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;

  LEOPPHAPI auto OnDrawProperties() -> void override;
};
}
