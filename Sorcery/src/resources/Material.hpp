#pragma once

#include "../Bounds.hpp"
#include "../Color.hpp"
#include "NativeResource.hpp"
#include "../Util.hpp"
#include "Texture2D.hpp"
#include "../ResourceManager.hpp"
#include "../rendering/shaders/shader_interop.h"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Material final : public NativeResource {
  RTTR_ENABLE(NativeResource)

public:
  enum class BlendMode : int {
    Opaque    = BLEND_MODE_OPAQUE,
    AlphaClip = BLEND_MODE_ALPHA_CLIP
  };

private:
  ShaderMaterial mShaderMtl{
    .albedo = Vector3{1, 1, 1}, .metallic = 0.0f, .roughness = 0.5f, .ao = 1.0f, .alphaThreshold = 1.0f,
    .albedo_map_idx = INVALID_RES_IDX, .metallic_map_idx = INVALID_RES_IDX, .roughness_map_idx = INVALID_RES_IDX,
    .ao_map_idx = INVALID_RES_IDX, .normal_map_idx = INVALID_RES_IDX, .opacity_map_idx = INVALID_RES_IDX,
    .blendMode = BLEND_MODE_OPAQUE
  };

  graphics::UniqueHandle<graphics::Buffer> cb_;

  ObserverPtr<Texture2D> albedo_map_{nullptr};
  ObserverPtr<Texture2D> metallic_map_{nullptr};
  ObserverPtr<Texture2D> roughness_map_{nullptr};
  ObserverPtr<Texture2D> ao_map_{nullptr};
  ObserverPtr<Texture2D> normal_map_{nullptr};
  ObserverPtr<Texture2D> opacity_mask_{nullptr};

  auto UpdateGPUData() const noexcept -> void;
  auto CreateCB() -> void;

public:
  LEOPPHAPI Material();

  [[nodiscard]] LEOPPHAPI auto GetAlbedoVector() const noexcept -> Vector3 const&;
  LEOPPHAPI auto SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAlbedoColor() const noexcept -> Color;
  LEOPPHAPI auto SetAlbedoColor(Color albedoColor) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMetallic() const noexcept -> f32;
  LEOPPHAPI auto SetMetallic(f32 metallic) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetRoughness() const noexcept -> f32;
  LEOPPHAPI auto SetRoughness(f32 roughness) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAo() const noexcept -> f32;
  LEOPPHAPI auto SetAo(f32 ao) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAlbedoMap() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetAlbedoMap(ObserverPtr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMetallicMap() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetMetallicMap(ObserverPtr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetRoughnessMap() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetRoughnessMap(ObserverPtr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAoMap() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetAoMap(ObserverPtr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormalMap() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetNormalMap(ObserverPtr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBlendMode() const noexcept -> BlendMode;
  LEOPPHAPI auto SetBlendMode(BlendMode blendMode) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAlphaThreshold() const noexcept -> float;
  LEOPPHAPI auto SetAlphaThreshold(float threshold) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetOpacityMask() const noexcept -> ObserverPtr<Texture2D>;
  LEOPPHAPI auto SetOpacityMask(ObserverPtr<Texture2D> opacityMask) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBuffer() const noexcept -> graphics::Buffer*;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
