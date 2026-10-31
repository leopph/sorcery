#include "sorcery/scene/environment_component.hpp"

#include "sorcery/world.hpp"
#include "sorcery/host/rendering/render_world.hpp"


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
}


// Reflection requires matching getter and setter types
// ReSharper disable once CppPassValueParameterByConstReference
auto EnvironmentComponent::SetSkyboxRefl(AssetRef<Cubemap> skybox) noexcept -> void {
  SetSkybox(skybox);
}
}
