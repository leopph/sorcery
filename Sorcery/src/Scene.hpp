#pragma once

#include "Entity.hpp"
#include "NativeResource.hpp"

#include "YamlInclude.hpp"

#include <memory>
#include <vector>
#include <span>


namespace sorcery {
class Scene : public NativeResource {
  RTTR_ENABLE(NativeResource)
  static Scene* sActiveScene;
  static std::vector<Scene*> sAllScenes;

  std::vector<std::unique_ptr<Entity>> mEntities;
  YAML::Node mYamlData;

public:
  [[nodiscard]] LEOPPHAPI static auto GetActiveScene() noexcept -> Scene*;

  LEOPPHAPI Scene();
  LEOPPHAPI ~Scene() override;

  LEOPPHAPI static Type const SerializationType;
  LEOPPHAPI auto GetSerializationType() const -> Type override;

  LEOPPHAPI auto CreateEntity() -> Entity&;
  LEOPPHAPI auto DestroyEntity(Entity const& entityToRemove) -> void;

  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const>;

  LEOPPHAPI auto Serialize(std::vector<std::uint8_t>& out) const noexcept -> void override;
  LEOPPHAPI auto Deserialize(std::span<std::uint8_t const> bytes) -> void;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load(ObjectInstantiatorManager const& manager) -> void;

  LEOPPHAPI auto Clear() -> void;
};
}
