#pragma once

#include "NativeResource.hpp"
#include "../Entity.hpp"
#include "../Component.hpp"

#include <memory>
#include <vector>
#include <span>
#include <concepts>


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

  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<ObserverPtr<Entity> const>;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load(ObjectInstantiatorManager const& manager) -> void;

  LEOPPHAPI auto Clear() -> void;

  LEOPPHAPI static Type const SerializationType;
  LEOPPHAPI auto GetSerializationType() const -> Type override;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;

  LEOPPHAPI auto OnDrawProperties() -> void override;
};
}
