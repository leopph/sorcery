#pragma once

#include <cstddef>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXTex.h>

#include "Resource.hpp"
#include "../observer_ptr.hpp"
#include "../resource_residency_policy.hpp"


namespace sorcery {
class Texture2D;


namespace detail {
auto ClearTex2DCpuData(Texture2D& tex) -> void;
}


class Texture2D final : public Resource {
  RTTR_ENABLE(Resource)

public:
  SORCERYAPI Texture2D(DirectX::ScratchImage image, CpuResidencyPolicy cpu_data_policy);
  Texture2D(Texture2D const& other) = delete;
  Texture2D(Texture2D&& other) noexcept = delete;

  ~Texture2D() override = default;

  auto operator=(Texture2D const& other) -> Texture2D& = delete;
  auto operator=(Texture2D&& other) noexcept -> Texture2D& = delete;

  [[nodiscard]] SORCERYAPI
  auto GetData() const -> ObserverPtr<DirectX::ScratchImage const>;

  [[nodiscard]] SORCERYAPI
  auto GetMetadata() const -> DirectX::TexMetadata const&;

  [[nodiscard]] SORCERYAPI
  auto GetCpuDataPolicy() const -> CpuResidencyPolicy;

  [[nodiscard]] SORCERYAPI
  auto GetWidth() const -> std::size_t;
  [[nodiscard]] SORCERYAPI
  auto GetHeight() const -> std::size_t;
  [[nodiscard]] SORCERYAPI
  auto GetChannelCount() const -> unsigned;

private:
  std::unique_ptr<DirectX::ScratchImage> data_;
  DirectX::TexMetadata metadata_;
  unsigned channel_count_;
  CpuResidencyPolicy cpu_policy_;

  friend auto detail::ClearTex2DCpuData(Texture2D& tex) -> void;
};
}
