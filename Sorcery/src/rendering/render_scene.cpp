#include "sorcery/host/rendering/render_scene.hpp"


namespace sorcery::rendering {
auto RenderScene::Register(StaticMeshComponent& static_mesh_component) noexcept -> void {
  static_mesh_components_.emplace_back(&static_mesh_component);
}


auto RenderScene::Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void {
  std::erase(static_mesh_components_, MakeObserver(&static_mesh_component));
}


auto RenderScene::Register(SkinnedMeshComponent& skinned_mesh_component) noexcept -> void {
  skinned_mesh_components_.emplace_back(&skinned_mesh_component);
}


auto RenderScene::Unregister(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void {
  std::erase(skinned_mesh_components_, MakeObserver(&skinned_mesh_component));
}


auto RenderScene::Register(LightComponent const& light_component) noexcept -> void {
  light_components_.emplace_back(&light_component);
}


auto RenderScene::Unregister(LightComponent const& light_component) noexcept -> void {
  std::erase(light_components_, MakeObserver(&light_component));
}


auto RenderScene::Register(Camera& cam) noexcept -> void {
  cameras_.emplace_back(&cam);
}


auto RenderScene::Unregister(Camera const& cam) noexcept -> void {
  std::erase(cameras_, MakeObserver(&cam));
}


auto RenderScene::GetStaticMeshComponents() const -> std::span<ObserverPtr<StaticMeshComponent> const> {
  return static_mesh_components_;
}


auto RenderScene::GetSkinnedMeshComponents() const -> std::span<ObserverPtr<SkinnedMeshComponent> const> {
  return skinned_mesh_components_;
}


auto RenderScene::GetLightComponents() const -> std::span<ObserverPtr<LightComponent const> const> {
  return light_components_;
}


auto RenderScene::GetCameras() const -> std::span<ObserverPtr<Camera> const> {
  return cameras_;
}
}
