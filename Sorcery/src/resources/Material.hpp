#pragma once

#include "NativeResource.hpp"
#include "Texture2D.hpp"
#include "../Color.hpp"
#include "../material_blend_mode.hpp"
#include "../resource_ref.hpp"
#include "../resource_residency_policy.hpp"
#include "../rendering/constant_buffer.hpp"
#include "../rendering/shaders/shader_interop.h"
#include "wand/buffer.hpp"
#include "wand/device_child.hpp"


namespace sorcery {
class Material final : public NativeResource {
  RTTR_ENABLE(NativeResource)
  RTTR_REGISTRATION_FRIEND

public:
  using BlendMode = MaterialBlendMode;

  [[nodiscard]] SORCERYAPI
  auto Serialize() const noexcept -> YAML::Node override;
  SORCERYAPI
  auto Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void override;

  SORCERYAPI explicit Material(GpuResidencyPolicy gpu_policy);
  Material(Material const&) = delete;
  Material(Material&&) noexcept = delete;

  SORCERYAPI ~Material() override;

  auto operator=(Material const&) -> void = delete;
  auto operator=(Material&&) noexcept -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoVector() const -> Vector3 const&;
  SORCERYAPI
  auto SetAlbedoVector(Vector3 const& albedo_vector, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoColor() const -> Color;
  SORCERYAPI
  auto SetAlbedoColor(Color albedo_color, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetMetallic() const -> f32;
  SORCERYAPI
  auto SetMetallic(f32 metallic, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRoughness() const -> f32;
  SORCERYAPI
  auto SetRoughness(f32 roughness, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAo() const -> f32;
  SORCERYAPI
  auto SetAo(f32 ao, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlbedoMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetAlbedoMap(ResourceRef<Texture2D> const& tex, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetMetallicMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetMetallicMap(ResourceRef<Texture2D> const& tex, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetRoughnessMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetRoughnessMap(ResourceRef<Texture2D> const& tex, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAoMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetAoMap(ResourceRef<Texture2D> const& tex, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetNormalMap() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetNormalMap(ResourceRef<Texture2D> const& tex, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetBlendMode() const -> BlendMode;
  SORCERYAPI
  auto SetBlendMode(BlendMode blend_mode, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetAlphaThreshold() const -> float;
  SORCERYAPI
  auto SetAlphaThreshold(float threshold, GpuResidencyPolicy gpu_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetOpacityMask() const -> ResourceRef<Texture2D>;
  SORCERYAPI
  auto SetOpacityMask(ResourceRef<Texture2D> const& opacity_mask, GpuResidencyPolicy gpu_policy) -> void;

  SORCERYAPI
  auto UploadToGpu() -> void;

  [[nodiscard]] SORCERYAPI
  auto GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;

private:
  // These are just for reflection and default to MakeResident

  auto SetAlbedoVectorRefl(Vector3 const& albedo_vector) -> void;
  auto SetMetallicRefl(f32 metallic) -> void;
  auto SetRoughnessRefl(f32 roughness) -> void;
  auto SetAoRefl(f32 ao) -> void;
  auto SetAlbedoMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetMetallicMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetRoughnessMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetAoMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetNormalMapRefl(ResourceRef<Texture2D> tex) -> void;
  auto SetBlendModeRefl(BlendMode blend_mode) -> void;
  auto SetAlphaThresholdRefl(float threshold) -> void;
  auto SetOpacityMaskRefl(ResourceRef<Texture2D> opacity_mask) -> void;

  ShaderMaterial cpu_data_{
    .albedo = Vector3{1, 1, 1}, .metallic = 0.0f, .roughness = 0.5f, .ao = 1.0f, .alphaThreshold = 1.0f,
    .albedo_map_idx = INVALID_RES_IDX, .metallic_map_idx = INVALID_RES_IDX, .roughness_map_idx = INVALID_RES_IDX,
    .ao_map_idx = INVALID_RES_IDX, .normal_map_idx = INVALID_RES_IDX, .opacity_map_idx = INVALID_RES_IDX,
    .blendMode = BLEND_MODE_OPAQUE
  };

  rendering::ConstantBuffer<ShaderMaterial> cb_;

  ResourceRef<Texture2D> albedo_map_{nullptr};
  ResourceRef<Texture2D> metallic_map_{nullptr};
  ResourceRef<Texture2D> roughness_map_{nullptr};
  ResourceRef<Texture2D> ao_map_{nullptr};
  ResourceRef<Texture2D> normal_map_{nullptr};
  ResourceRef<Texture2D> opacity_mask_{nullptr};
};
}
