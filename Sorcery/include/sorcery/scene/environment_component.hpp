#pragma once

#include <wand/device_object.hpp>
#include <wand/texture.hpp>

#include <sorcery/asset_ref.hpp>
#include <sorcery/Color.hpp>
#include <sorcery/Math.hpp>
#include <sorcery/sky_mode.hpp>
#include <sorcery/assets/Cubemap.hpp>
#include <sorcery/scene/Component.hpp>


namespace sorcery {
class EnvironmentComponent;


namespace detail {
[[nodiscard]]
auto GetIrradianceMap(EnvironmentComponent const& env) -> wand::SharedDeviceHandle<wand::Texture> const&;
auto RecreateIrradianceMap(EnvironmentComponent& env, wand::GraphicsDevice& device, DXGI_FORMAT format,
                           UINT size) -> void;

[[nodiscard]]
auto GetPrefilteredEnvMap(EnvironmentComponent const& env) -> wand::SharedDeviceHandle<wand::Texture> const&;
auto RecreatePrefilteredEnvMap(EnvironmentComponent& env, wand::GraphicsDevice& device, DXGI_FORMAT format,
                               UINT size) -> void;
}


class EnvironmentComponent : public Component {
public:
  [[nodiscard]] SORCERYAPI
  auto Clone() -> std::unique_ptr<SceneObject> override;
  SORCERYAPI
  auto OnAfterEnteringWorld(World& world) -> void override;
  SORCERYAPI
  auto OnBeforeExitingWorld(World& world) -> void override;

  [[nodiscard]] SORCERYAPI auto GetAmbientLightVector() const -> Vector3 const&;
  SORCERYAPI auto SetAmbientLightVector(Vector3 const& vector) -> void;

  [[nodiscard]] SORCERYAPI auto GetAmbientLight() const -> Color;
  SORCERYAPI auto SetAmbientLight(Color const& color) -> void;

  [[nodiscard]] SORCERYAPI auto GetSkyMode() const -> SkyMode;
  SORCERYAPI auto SetSkyMode(SkyMode sky_mode) -> void;

  [[nodiscard]] SORCERYAPI auto GetSkyColor() const -> Vector3 const&;
  SORCERYAPI auto SetSkyColor(Vector3 const& sky_color) -> void;

  [[nodiscard]] SORCERYAPI auto GetSkybox() const -> AssetRef<Cubemap>;
  SORCERYAPI auto SetSkybox(AssetRef<Cubemap> const& skybox) -> void;

private:
  auto SetSkyboxRefl(AssetRef<Cubemap> skybox) noexcept -> void;

  Vector3 ambient_light_{20.0f / 255.0f};

  AssetRef<Cubemap> skybox_{nullptr};
  SkyMode sky_mode_{SkyMode::kColor};
  Vector3 sky_color_{0.F, 36.F / 255.F, 1.F};
  wand::SharedDeviceHandle<wand::Texture> irradiance_map_{};
  wand::SharedDeviceHandle<wand::Texture> prefiltered_env_map_{};

  RTTR_ENABLE(Component)
  RTTR_REGISTRATION_FRIEND

  friend auto detail::GetIrradianceMap(EnvironmentComponent const& env)
    -> wand::SharedDeviceHandle<wand::Texture> const&;
  friend auto detail::RecreateIrradianceMap(EnvironmentComponent& env, wand::GraphicsDevice& device, DXGI_FORMAT format,
                                            UINT size) -> void;

  friend auto detail::GetPrefilteredEnvMap(EnvironmentComponent const& env)
    -> wand::SharedDeviceHandle<wand::Texture> const&;
  friend auto detail::RecreatePrefilteredEnvMap(EnvironmentComponent& env, wand::GraphicsDevice& device,
                                                DXGI_FORMAT format, UINT size) -> void;
};
}
