#include "sorcery/scene/environment_component.hpp"

#include "sorcery/world.hpp"
#include "sorcery/host/rendering/render_world.hpp"
#include "wand/wand.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::EnvironmentComponent>{"Environment Component"}
    .REFLECT_REGISTER_SCENE_OBJECT_CTOR
    .property("ambientLight", &sorcery::EnvironmentComponent::GetAmbientLightVector,
      &sorcery::EnvironmentComponent::SetAmbientLightVector)
    .property("skyMode", &sorcery::EnvironmentComponent::GetSkyMode, &sorcery::EnvironmentComponent::SetSkyMode)
    .property("skyColor", &sorcery::EnvironmentComponent::GetSkyColor, &sorcery::EnvironmentComponent::SetSkyColor)
    .property("skybox", &sorcery::EnvironmentComponent::GetSkybox, &sorcery::EnvironmentComponent::SetSkyboxRefl);
}


namespace sorcery {
auto detail::GetIrradianceMap(EnvironmentComponent const& env) -> wand::SharedDeviceHandle<wand::Texture> const& {
  return env.irradiance_map_;
}


auto detail::RecreateIrradianceMap(EnvironmentComponent& env, wand::GraphicsDevice& device, DXGI_FORMAT const format,
                                   UINT const size) -> void {
  if (!env.skybox_) {
    return;
  }

  env.irradiance_map_ = device.CreateTexture(wand::TextureDesc{
      .dimension = wand::TextureDimension::kCube,
      .width = size,
      .height = size,
      .depth_or_array_size = 6,
      .mip_levels = 1,
      .format = format,
      .sample_count = 1,
      .depth_stencil = false,
      .render_target = true,
      .shader_resource = true,
      .unordered_access = false
    }, wand::CpuAccess::kNone,
    std::array{D3D12_CLEAR_VALUE{.Format = format, .Color = {0.0F, 0.0F, 0.0F, 1.0F}}}.data());
}


auto detail::GetPrefilteredEnvMap(EnvironmentComponent const& env) -> wand::SharedDeviceHandle<wand::Texture> const& {
  return env.prefiltered_env_map_;
}


auto detail::RecreatePrefilteredEnvMap(EnvironmentComponent& env, wand::GraphicsDevice& device,
                                       DXGI_FORMAT const format, UINT const size) -> void {
  env.prefiltered_env_map_ = device.CreateTexture(wand::TextureDesc{
      .dimension = wand::TextureDimension::kCube,
      .width = size,
      .height = size,
      .depth_or_array_size = 6,
      .mip_levels = 0,
      .format = format,
      .sample_count = 1,
      .depth_stencil = false,
      .render_target = true,
      .shader_resource = true,
      .unordered_access = false
    }, wand::CpuAccess::kNone,
    std::array{D3D12_CLEAR_VALUE{.Format = format, .Color = {0.0F, 0.0F, 0.0F, 1.0F}}}.data());
}


auto EnvironmentComponent::Clone() -> std::unique_ptr<SceneObject> {
  return std::make_unique<EnvironmentComponent>(*this);
}


auto EnvironmentComponent::OnAfterEnteringWorld(World& world) -> void {
  Component::OnAfterEnteringWorld(world);
  world.GetRenderWorld().Register(*this);
}


auto EnvironmentComponent::OnBeforeExitingWorld(World& world) -> void {
  world.GetRenderWorld().Unregister(*this);
  Component::OnBeforeExitingWorld(world);
}


auto EnvironmentComponent::GetAmbientLightVector() const -> Vector3 const& {
  return ambient_light_;
}


auto EnvironmentComponent::SetAmbientLightVector(Vector3 const& vector) -> void {
  ambient_light_ = vector;
}


auto EnvironmentComponent::GetAmbientLight() const -> Color {
  return Color{Vector4{ambient_light_, 1}};
}


auto EnvironmentComponent::SetAmbientLight(Color const& color) -> void {
  ambient_light_ = Vector3{static_cast<Vector4>(color)};
}


auto EnvironmentComponent::GetSkyMode() const -> SkyMode {
  return sky_mode_;
}


auto EnvironmentComponent::SetSkyMode(SkyMode const sky_mode) -> void {
  sky_mode_ = sky_mode;
}


auto EnvironmentComponent::GetSkyColor() const -> Vector3 const& {
  return sky_color_;
}


auto EnvironmentComponent::SetSkyColor(Vector3 const& sky_color) -> void {
  sky_color_ = sky_color;
}


auto EnvironmentComponent::GetSkybox() const -> AssetRef<Cubemap> {
  return skybox_;
}


auto EnvironmentComponent::SetSkybox(AssetRef<Cubemap> const& skybox) -> void {
  skybox_ = skybox;
  irradiance_map_ = nullptr;
  prefiltered_env_map_ = nullptr;
}


// Reflection requires matching getter and setter types
// ReSharper disable once CppPassValueParameterByConstReference
auto EnvironmentComponent::SetSkyboxRefl(AssetRef<Cubemap> skybox) noexcept -> void {
  SetSkybox(skybox);
}
}
