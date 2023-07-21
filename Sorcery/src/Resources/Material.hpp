#pragma once

#include "../Bounds.hpp"
#include "../Color.hpp"
#include "NativeResource.hpp"
#include "../Util.hpp"
#include "Texture2D.hpp"

#include "../shaders/ShaderInterop.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>
#include <wrl/client.h>


namespace sorcery {
class Material final : public NativeResource {
  RTTR_ENABLE(NativeResource)
  ShaderMaterial mShaderMtl{
    .albedo = Vector3{ 1, 1, 1 },
    .metallic = 0.0f,
    .roughness = 0.5f,
    .ao = 1.0f,
    .sampleAlbedo = FALSE,
    .sampleMetallic = FALSE,
    .sampleRoughness = FALSE,
    .sampleAo = FALSE,
    .sampleNormal = FALSE
  };

  Microsoft::WRL::ComPtr<ID3D11Buffer> mCB;

  std::weak_ptr<Texture2D> mAlbedoMap{};
  std::weak_ptr<Texture2D> mMetallicMap{};
  std::weak_ptr<Texture2D> mRoughnessMap{};
  std::weak_ptr<Texture2D> mAoMap{};
  std::weak_ptr<Texture2D> mNormalMap{};

  auto UpdateGPUData() const noexcept -> void;
  auto CreateCB() -> void;

public:
  LEOPPHAPI Material();
  LEOPPHAPI Material(Vector3 const& albedoVector, float metallic, float roughness, float ao, std::weak_ptr<Texture2D> albedoMap, std::weak_ptr<Texture2D> metallicMap, std::weak_ptr<Texture2D> roughnessMap, std::weak_ptr<Texture2D> aoMap, std::weak_ptr<Texture2D> normalMap);

  [[nodiscard]] LEOPPHAPI auto GetAlbedoVector() const noexcept -> Vector3;
  LEOPPHAPI auto SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAlbedoColor() const noexcept -> Color;
  LEOPPHAPI auto SetAlbedoColor(Color const& albedoColor) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMetallic() const noexcept -> f32;
  LEOPPHAPI auto SetMetallic(f32 metallic) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetRoughness() const noexcept -> f32;
  LEOPPHAPI auto SetRoughness(f32 roughness) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAo() const noexcept -> f32;
  LEOPPHAPI auto SetAo(f32 ao) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAlbedoMap() const noexcept -> std::weak_ptr<Texture2D>;
  LEOPPHAPI auto SetAlbedoMap(std::weak_ptr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMetallicMap() const noexcept -> std::weak_ptr<Texture2D>;
  LEOPPHAPI auto SetMetallicMap(std::weak_ptr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetRoughnessMap() const noexcept -> std::weak_ptr<Texture2D>;
  LEOPPHAPI auto SetRoughnessMap(std::weak_ptr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAoMap() const noexcept -> std::weak_ptr<Texture2D>;
  LEOPPHAPI auto SetAoMap(std::weak_ptr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormalMap() const noexcept -> std::weak_ptr<Texture2D>;
  LEOPPHAPI auto SetNormalMap(std::weak_ptr<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBuffer() const noexcept -> ObserverPtr<ID3D11Buffer>;

  LEOPPHAPI static Type const SerializationType;
  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;
};
}
