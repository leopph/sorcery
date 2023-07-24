#pragma once

#include "../Bounds.hpp"
#include "../Color.hpp"
#include "NativeResource.hpp"
#include "../Util.hpp"
#include "Texture2D.hpp"
#include "../ResourceManager.hpp"

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

  ResourceHandle<Texture2D> mAlbedoMap;
  ResourceHandle<Texture2D> mMetallicMap;
  ResourceHandle<Texture2D> mRoughnessMap;
  ResourceHandle<Texture2D> mAoMap;
  ResourceHandle<Texture2D> mNormalMap;

  auto UpdateGPUData() const noexcept -> void;
  auto CreateCB() -> void;

public:
  LEOPPHAPI Material();

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

  [[nodiscard]] LEOPPHAPI auto GetAlbedoMap() const noexcept -> ResourceHandle<Texture2D>;
  LEOPPHAPI auto SetAlbedoMap(ResourceHandle<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMetallicMap() const noexcept -> ResourceHandle<Texture2D>;
  LEOPPHAPI auto SetMetallicMap(ResourceHandle<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetRoughnessMap() const noexcept -> ResourceHandle<Texture2D>;
  LEOPPHAPI auto SetRoughnessMap(ResourceHandle<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetAoMap() const noexcept -> ResourceHandle<Texture2D>;
  LEOPPHAPI auto SetAoMap(ResourceHandle<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetNormalMap() const noexcept -> ResourceHandle<Texture2D>;
  LEOPPHAPI auto SetNormalMap(ResourceHandle<Texture2D> tex) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetBuffer() const noexcept -> ObserverPtr<ID3D11Buffer>;

  LEOPPHAPI static Type const SerializationType;
  [[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;

  [[nodiscard]] LEOPPHAPI auto Serialize() const noexcept -> YAML::Node override;
  LEOPPHAPI auto Deserialize(YAML::Node const& yamlNode) noexcept -> void override;
};
}
