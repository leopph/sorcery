#pragma once

#include <cstdint>

#include <sorcery/asset_ref.hpp>
#include <sorcery/Color.hpp>
#include <sorcery/material_blend_mode.hpp>
#include <sorcery/assets/NativeAsset.hpp>
#include <sorcery/assets/Texture2D.hpp>


namespace sorcery {
class Material final : public NativeAsset {
  RTTR_ENABLE(NativeAsset)
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
  auto GetEmissiveVector() const -> Vector3 const&;
  SORCERYAPI
  auto SetEmissiveVector(Vector3 const& emissive_vector) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetEmissiveColor() const -> Color;
  SORCERYAPI
  auto SetEmissiveColor(Color emissive_color) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetEmissiveIntensity() const -> float;
  SORCERYAPI
  auto SetEmissiveIntensity(float intensity) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetAlbedoMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetMetallicMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetMetallicMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRoughnessMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetRoughnessMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAoMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetAoMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetNormalMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetNormalMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetBlendMode() const -> MaterialBlendMode;
  SORCERYAPI
  auto SetBlendMode(MaterialBlendMode blend_mode) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlphaThreshold() const -> float;
  SORCERYAPI
  auto SetAlphaThreshold(float threshold) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetOpacityMask() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetOpacityMask(AssetRef<Texture2D> const& opacity_mask) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetEmissiveMap() const -> AssetRef<Texture2D>;
  SORCERYAPI
  auto SetEmissiveMap(AssetRef<Texture2D> const& tex) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRevision() const -> std::uint64_t;

private:
  auto SetAlbedoMapRefl(AssetRef<Texture2D> tex) -> void;
  auto SetMetallicMapRefl(AssetRef<Texture2D> tex) -> void;
  auto SetRoughnessMapRefl(AssetRef<Texture2D> tex) -> void;
  auto SetAoMapRefl(AssetRef<Texture2D> tex) -> void;
  auto SetNormalMapRefl(AssetRef<Texture2D> tex) -> void;
  auto SetBlendModeRefl(MaterialBlendMode blend_mode) -> void;
  auto SetAlphaThresholdRefl(float threshold) -> void;
  auto SetOpacityMaskRefl(AssetRef<Texture2D> opacity_mask) -> void;
  auto SetEmissiveMapRefl(AssetRef<Texture2D> tex) -> void;

  Vector3 albedo_{1.0f, 1.0f, 1.0f};
  float metallic_{0.0f};
  float roughness_{0.5f};
  float ao_{1.0f};
  float alpha_threshold_{1.0f};
  Vector3 emissive_{0.0f, 0.0f, 0.0f};
  float emissive_intensity_{1.0f};

  MaterialBlendMode blend_mode_{MaterialBlendMode::kOpaque};

  AssetRef<Texture2D> albedo_map_{nullptr};
  AssetRef<Texture2D> metallic_map_{nullptr};
  AssetRef<Texture2D> roughness_map_{nullptr};
  AssetRef<Texture2D> ao_map_{nullptr};
  AssetRef<Texture2D> normal_map_{nullptr};
  AssetRef<Texture2D> opacity_mask_{nullptr};
  AssetRef<Texture2D> emissive_map_{nullptr};

  std::uint64_t revision_{0};
};
}
