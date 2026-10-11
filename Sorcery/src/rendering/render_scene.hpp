#pragma once

#include <span>
#include <vector>

#include "../Core.hpp"
#include "../observer_ptr.hpp"


namespace sorcery {
class StaticMeshComponent;
class SkinnedMeshComponent;
class LightComponent;


namespace rendering {
class Camera;


class RenderScene {
public:
  SORCERYAPI
  auto Register(StaticMeshComponent& static_mesh_component) noexcept -> void;
  SORCERYAPI
  auto Unregister(StaticMeshComponent const& static_mesh_component) noexcept -> void;

  SORCERYAPI
  auto Register(SkinnedMeshComponent& skinned_mesh_component) noexcept -> void;
  SORCERYAPI
  auto Unregister(SkinnedMeshComponent const& skinned_mesh_component) noexcept -> void;

  SORCERYAPI
  auto Register(LightComponent const& light_component) noexcept -> void;
  SORCERYAPI
  auto Unregister(LightComponent const& light_component) noexcept -> void;

  SORCERYAPI
  auto Register(Camera& cam) noexcept -> void;
  SORCERYAPI
  auto Unregister(Camera const& cam) noexcept -> void;

  [[nodiscard]]
  auto GetStaticMeshComponents() const -> std::span<ObserverPtr<StaticMeshComponent> const>;
  [[nodiscard]]
  auto GetSkinnedMeshComponents() const -> std::span<ObserverPtr<SkinnedMeshComponent> const>;
  [[nodiscard]]
  auto GetLightComponents() const -> std::span<ObserverPtr<LightComponent const> const>;
  [[nodiscard]]
  auto GetCameras() const -> std::span<ObserverPtr<Camera> const>;

  RenderScene() = default;
  RenderScene(RenderScene const& other) = delete;
  RenderScene(RenderScene&& other) noexcept = delete;

  ~RenderScene() = default;

  auto operator=(RenderScene const& other) -> RenderScene& = delete;
  auto operator=(RenderScene&& other) noexcept -> RenderScene& = delete;

private:
  std::vector<ObserverPtr<StaticMeshComponent>> static_mesh_components_;
  std::vector<ObserverPtr<SkinnedMeshComponent>> skinned_mesh_components_;
  std::vector<ObserverPtr<LightComponent const>> light_components_;
  std::vector<ObserverPtr<Camera>> cameras_;
};
}
}
