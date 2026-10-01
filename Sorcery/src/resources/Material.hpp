#pragma once

#include <cstdint>

#include "NativeResource.hpp"
#include "Texture2D.hpp"
#include "../Color.hpp"
#include "../material_blend_mode.hpp"
#include "../resource_ref.hpp"


namespace sorcery {
class Material final : public NativeResource {
  RTTR_ENABLE(NativeResource)
  RTTR_REGISTRATION_FRIEND

public:
  [[nodiscard]] SORCERYAPI
  auto Serialize() const noexcept -> YAML::Node override;
  SORCERYAPI
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

  Material() = default;
  Material(Material const&) = delete;
  Material(Material&&) noexcept = delete;

  ~Material() override = default;

  auto operator=(Material const&) -> void = delete;
  auto operator=(Material&&) noexcept -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoVector() const -> Vector3 const&;
  SORCERYAPI
  auto SetAlbedoVector(Vector3 const& albedo_vector) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoColor() const -> Color;
  SORCERYAPI
  auto SetAlbedoColor(Color albedo_color) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetMetallic() const -> f32;
  SORCERYAPI
  auto SetMetallic(f32 metallic) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRoughness() const -> f32;
  SORCERYAPI
  auto SetRoughness(f32 roughness) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAo() const -> f32;
  SORCERYAPI
  auto SetAo(f32 ao) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetAlbedoMap(ResourceRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetMetallicMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetMetallicMap(ResourceRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRoughnessMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetRoughnessMap(ResourceRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAoMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetAoMap(ResourceRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetNormalMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetNormalMap(ResourceRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetBlendMode() const -> MaterialBlendMode;
  SORCERYAPI
  auto SetBlendMode(MaterialBlendMode blend_mode) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlphaThreshold() const -> float;
  SORCERYAPI
  auto SetAlphaThreshold(float threshold) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetOpacityMask() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetOpacityMask(ResourceRef<Texture2D> const& opacity_mask) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRevision() const -> std::uint64_t;

private:
  auto SetAlbedoMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetMetallicMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetRoughnessMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetAoMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetNormalMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetBlendModeRefl(MaterialBlendMode blend_mode) -> void;
  auto SetAlphaThresholdRefl(float threshold) -> void;
  auto SetOpacityMaskRefl(ResourceRef<Texture2D> opacity_mask) -> void;

  Vector3 albedo_{1.0f, 1.0f, 1.0f};
  float metallic_{0.0f};
  float roughness_{0.5f};
  float ao_{1.0f};
  float alpha_threshold_{1.0f};

  MaterialBlendMode blend_mode_{MaterialBlendMode::kOpaque};

  ResourceRef<Texture2D> albedo_map_{nullptr};
  ResourceRef<Texture2D> metallic_map_{nullptr};
  ResourceRef<Texture2D> roughness_map_{nullptr};
  ResourceRef<Texture2D> ao_map_{nullptr};
  ResourceRef<Texture2D> normal_map_{nullptr};
  ResourceRef<Texture2D> opacity_mask_{nullptr};

  std::uint64_t revision_{0};
};
}
