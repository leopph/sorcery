#pragma once

#include <vector>

#include <wand/device_object.hpp>
#include <wand/texture.hpp>

#include <sorcery/asset_ref.hpp>
#include <sorcery/Color.hpp>
#include <sorcery/SkyMode.hpp>
#include <sorcery/assets/Cubemap.hpp>
#include <sorcery/assets/NativeAsset.hpp>


namespace sorcery {
class Scene;
class World;


namespace detail {
[[nodiscard]]
auto GetIrradianceMap(Scene const& scene) -> wand::SharedDeviceHandle<wand::Texture> const&;
auto RecreateIrradianceMap(Scene& scene, wand::GraphicsDevice& device, DXGI_FORMAT format, UINT size) -> void;

[[nodiscard]]
auto GetPrefilteredEnvMap(Scene const& scene) -> wand::SharedDeviceHandle<wand::Texture> const&;
auto RecreatePrefilteredEnvMap(Scene& scene, wand::GraphicsDevice& device, DXGI_FORMAT format, UINT size) -> void;
}


class Scene final : public NativeAsset {
  RTTR_ENABLE(NativeAsset)
  RTTR_REGISTRATION_FRIEND

  friend auto detail::GetIrradianceMap(Scene const& scene)
    -> wand::SharedDeviceHandle<wand::Texture> const&;
  friend auto detail::RecreateIrradianceMap(Scene& scene, wand::GraphicsDevice& device, DXGI_FORMAT format,
                                            UINT size) -> void;

  friend auto detail::GetPrefilteredEnvMap(Scene const& scene)
    -> wand::SharedDeviceHandle<wand::Texture> const&;
  friend auto detail::RecreatePrefilteredEnvMap(Scene& scene, wand::GraphicsDevice& device, DXGI_FORMAT format,
                                                UINT size) -> void;

public:
  // The active scene is the one that other systems take global information (such as sky settings) from.
  [[nodiscard]] SORCERYAPI static auto GetActiveScene() noexcept -> Scene*;

  SORCERYAPI Scene();
  Scene(Scene const& other) = delete;
  Scene(Scene&& other) = delete;

  SORCERYAPI ~Scene() override;

  auto operator=(Scene const& other) -> void = delete;
  auto operator=(Scene&& other) -> void = delete;

  SORCERYAPI auto Save(World const& world) -> void;
  SORCERYAPI auto Load(World& world) -> void;
  SORCERYAPI auto SetActive() -> void;

  [[nodiscard]] SORCERYAPI auto Serialize() const noexcept -> YAML::Node override;
  SORCERYAPI auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

  [[nodiscard]] SORCERYAPI auto GetAmbientLightVector() const noexcept -> Vector3 const&;
  SORCERYAPI auto SetAmbientLightVector(Vector3 const& vector) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetAmbientLight() const noexcept -> Color;
  SORCERYAPI auto SetAmbientLight(Color const& color) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetSkyMode() const noexcept -> SkyMode;
  SORCERYAPI auto SetSkyMode(SkyMode skyMode) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetSkyColor() const noexcept -> Vector3 const&;
  SORCERYAPI auto SetSkyColor(Vector3 const& skyColor) noexcept -> void;

  [[nodiscard]] SORCERYAPI auto GetSkybox() const noexcept -> AssetRef<Cubemap>;
  SORCERYAPI auto SetSkybox(AssetRef<Cubemap> const& skybox) noexcept -> void;

private:
  // This is just for reflection
  auto SetSkyboxRefl(AssetRef<Cubemap> skybox) noexcept -> void;

  auto SaveSceneParams() -> void;

  static Scene* active_scene_;
  static std::vector<Scene*> all_scenes_;

  YAML::Node yaml_data_;
  YamlDeserializeContext yaml_ctx_;

  Vector3 ambient_light_{20.0f / 255.0f};

  AssetRef<Cubemap> skybox_{nullptr};
  SkyMode sky_mode_{SkyMode::Color};
  Vector3 sky_color_{0.F, 36.F / 255.F, 1.F};
  wand::SharedDeviceHandle<wand::Texture> irradiance_map_{};
  wand::SharedDeviceHandle<wand::Texture> prefiltered_env_map_{};
};
}
