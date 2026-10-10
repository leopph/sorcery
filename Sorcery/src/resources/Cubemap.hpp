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
class Cubemap;


namespace detail {
auto ClearCubemapCpuData(Cubemap& cubemap) -> void;
}


class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)

public:
  SORCERYAPI Cubemap(DirectX::ScratchImage image, CpuResidencyPolicy cpu_data_policy);
  Cubemap(Cubemap const& other) = delete;
  Cubemap(Cubemap&& other) noexcept = delete;

  ~Cubemap() override = default;

  auto operator=(Cubemap const& other) -> Cubemap& = delete;
  auto operator=(Cubemap&& other) noexcept -> Cubemap& = delete;

  [[nodiscard]] SORCERYAPI
  auto GetData() const -> ObserverPtr<DirectX::ScratchImage const>;

  [[nodiscard]] SORCERYAPI
  auto GetMetadata() const -> DirectX::TexMetadata const&;

  [[nodiscard]] SORCERYAPI
  auto GetCpuDataPolicy() const -> CpuResidencyPolicy;

private:
  std::unique_ptr<DirectX::ScratchImage> data_;
  DirectX::TexMetadata metadata_;
  CpuResidencyPolicy cpu_policy_;

  friend auto detail::ClearCubemapCpuData(Cubemap& cubemap) -> void;
};
}
