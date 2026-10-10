#include "Texture2D.hpp"

#include <utility>


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
  channel_count_{
    DirectX::IsCompressed(metadata_.format)
      ? [](DXGI_FORMAT const format) {
        switch (DirectX::MakeTypeless(format)) {
          case DXGI_FORMAT_BC1_TYPELESS:
            return 3;
          case DXGI_FORMAT_BC2_TYPELESS:
            [[fallthrough]];
          case DXGI_FORMAT_BC3_TYPELESS:
            return 4;
          case DXGI_FORMAT_BC4_TYPELESS:
            return 1;
          case DXGI_FORMAT_BC5_TYPELESS:
            return 2;
          case DXGI_FORMAT_BC6H_TYPELESS:
            return 3;
          case DXGI_FORMAT_BC7_TYPELESS:
            return 4;
          default:
            return 0;
        }
      }(metadata_.format)
      : static_cast<unsigned>(DirectX::BitsPerPixel(metadata_.format) / DirectX::BitsPerColor(metadata_.format))
  },
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
