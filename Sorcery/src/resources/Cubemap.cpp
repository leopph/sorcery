#include "Cubemap.hpp"

#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Cubemap>{"Cubemap"};
}


namespace sorcery {
auto detail::ClearCubemapCpuData(Cubemap& cubemap) -> void {
  cubemap.data_.reset();
}


Cubemap::Cubemap(DirectX::ScratchImage image, CpuResidencyPolicy const cpu_data_policy) :
  data_{std::make_unique<DirectX::ScratchImage>(std::move(image))},
  metadata_{data_->GetMetadata()},
  cpu_policy_{cpu_data_policy} {}


auto Cubemap::GetData() const -> ObserverPtr<DirectX::ScratchImage const> {
  return MakeObserver(data_.get());
}


auto Cubemap::GetMetadata() const -> DirectX::TexMetadata const& {
  return metadata_;
}


auto Cubemap::GetCpuDataPolicy() const -> CpuResidencyPolicy {
  return cpu_policy_;
}
}
