#pragma once

#include "Cubemap.hpp"
#include "NativeResource.hpp"
#include "../Color.hpp"
#include "../SkyMode.hpp"
#include "../scene_objects/Entity.hpp"

#include <vector>


namespace sorcery {
class Scene final : public NativeResource {
  RTTR_ENABLE(NativeResource)

public:
  // The active scene is the one that other systems take global information (such as sky settings) from.
  [[nodiscard]] LEOPPHAPI static auto GetActiveScene() noexcept -> Scene*;

  LEOPPHAPI Scene();
  Scene(Scene const& other) = delete;
  Scene(Scene&& other) = delete;

  LEOPPHAPI ~Scene() override;

  auto operator=(Scene const& other) -> void = delete;
  auto operator=(Scene&& other) -> void = delete;

  LEOPPHAPI auto AddEntity(std::unique_ptr<Entity> entity) -> void;
  LEOPPHAPI auto RemoveEntity(Entity const& entity) -> std::unique_ptr<Entity>;
  [[nodiscard]] LEOPPHAPI auto GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const>;

  LEOPPHAPI auto Save() -> void;
  LEOPPHAPI auto Load() -> void;
  LEOPPHAPI auto SetActive() -> void;
  LEOPPHAPI auto Clear() -> void;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;

  [[nodiscard]] LEOPPHAPI auto GetAmbientLightVector() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetAmbientLightVector(Vector3 const& vector) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAmbientLight() const noexcept -> Color;
  LEOPPHAPI auto SetAmbientLight(Color const& color) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSkyMode() const noexcept -> SkyMode;
  LEOPPHAPI auto SetSkyMode(SkyMode skyMode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSkyColor() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetSkyColor(Vector3 const& skyColor) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetSkybox() const noexcept -> Cubemap*;
  LEOPPHAPI auto SetSkybox(Cubemap* skybox) noexcept -> void;

private:
  static Scene* active_scene_;
  static std::vector<Scene*> all_scenes_;

  std::vector<std::unique_ptr<Entity>> entities_;

  YAML::Node yaml_data_;

  Vector3 ambient_light_{20.0f / 255.0f};

  Cubemap* skybox_{nullptr};
  SkyMode sky_mode_{SkyMode::Color};
  Vector3 sky_color_{10.0f / 255.0f};
};
}
