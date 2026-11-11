#include "sorcery/assets/Texture2D.hpp"

#include <utility>

#include "sorcery/rendering/dxgi_format.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{"Texture2D"};
}


namespace sorcery {
auto detail::ClearTex2DCpuData(Texture2D& tex) -> void {
  tex.data_.reset();
}


Texture2D::Texture2D(DirectX::ScratchImage image, CpuResidencyPolicy const cpu_data_policy) :
  data_{std::make_unique<DirectX::ScratchImage>(std::move(image))},
  metadata_{data_->GetMetadata()},
  channel_count_{rendering::GetFormatChannelCount(metadata_.format)},
  cpu_policy_{cpu_data_policy} {}


auto Texture2D::GetData() const -> ObserverPtr<DirectX::ScratchImage const> {
  return MakeObserver(data_.get());
}


auto Texture2D::GetMetadata() const -> DirectX::TexMetadata const& {
  return metadata_;
}


auto Texture2D::GetCpuDataPolicy() const -> CpuResidencyPolicy {
  return cpu_policy_;
}


auto Texture2D::GetWidth() const -> std::size_t {
  return metadata_.width;
}


auto Texture2D::GetHeight() const -> std::size_t {
  return metadata_.height;
}


auto Texture2D::GetChannelCount() const -> unsigned {
  return channel_count_;
}
}
