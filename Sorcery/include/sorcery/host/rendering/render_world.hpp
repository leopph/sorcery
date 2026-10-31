#pragma once

#include <span>
#include <vector>

#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>


namespace sorcery {
class StaticMeshComponent;
class SkinnedMeshComponent;
class LightComponent;
class EnvironmentComponent;


namespace rendering {
class RenderWorld {
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
  auto Register(EnvironmentComponent& env_component) noexcept -> void;
  SORCERYAPI
  auto Unregister(EnvironmentComponent const& env_component) noexcept -> void;

  [[nodiscard]]
  auto GetStaticMeshComponents() const -> std::span<ObserverPtr<StaticMeshComponent> const>;
  [[nodiscard]]
  auto GetSkinnedMeshComponents() const -> std::span<ObserverPtr<SkinnedMeshComponent> const>;
  [[nodiscard]]
  auto GetLightComponents() const -> std::span<ObserverPtr<LightComponent const> const>;
  [[nodiscard]]
  auto GetEnvironmentComponent() const -> ObserverPtr<EnvironmentComponent>;

  RenderWorld() = default;
  RenderWorld(RenderWorld const& other) = delete;
  RenderWorld(RenderWorld&& other) noexcept = delete;

  ~RenderWorld() = default;

  auto operator=(RenderWorld const& other) -> RenderWorld& = delete;
  auto operator=(RenderWorld&& other) noexcept -> RenderWorld& = delete;

private:
  std::vector<ObserverPtr<StaticMeshComponent>> static_mesh_components_;
  std::vector<ObserverPtr<SkinnedMeshComponent>> skinned_mesh_components_;
  std::vector<ObserverPtr<LightComponent const>> light_components_;
  std::vector<ObserverPtr<EnvironmentComponent>> environment_components_;
};
}
}
