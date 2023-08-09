#include "Texture2D.hpp"

#include <imgui.h>

#include "../Util.hpp"
#include "../Renderer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{"Texture2D"};
}


namespace sorcery {
auto Texture2D::UploadToGpu(bool const allowBlockCompression) -> void {
  struct FormatInfo {
    DXGI_FORMAT texFormat;
    DXGI_FORMAT srvFormat;
    std::optional<Image::BlockCompressedData> compressedData;
  };

  auto const& [texFormat, srvFormat, alternativeData]{
    [this, allowBlockCompression]() -> FormatInfo {
      auto const channelCount{mImgData->GetChannelCount()};

      if (allowBlockCompression) {
        if (auto blockCompressedData{mImgData->CreateBlockCompressedData()}) {
          if (channelCount == 1) {
            return FormatInfo{.texFormat = DXGI_FORMAT_BC4_TYPELESS, .srvFormat = DXGI_FORMAT_BC4_UNORM, .compressedData = std::move(blockCompressedData)};
          }
          if (channelCount == 2) {
            return FormatInfo{.texFormat = DXGI_FORMAT_BC5_TYPELESS, .srvFormat = DXGI_FORMAT_BC5_UNORM, .compressedData = std::move(blockCompressedData)};
          }
          if (channelCount == 3) {
            if (mImgData->IsSrgb()) {
              return FormatInfo{.texFormat = DXGI_FORMAT_BC1_TYPELESS, .srvFormat = DXGI_FORMAT_BC1_UNORM_SRGB, .compressedData = std::move(blockCompressedData)};
            }
            return FormatInfo{.texFormat = DXGI_FORMAT_BC1_TYPELESS, .srvFormat = DXGI_FORMAT_BC1_UNORM, .compressedData = std::move(blockCompressedData)};
          }
          if (channelCount == 4) {
            if (mImgData->IsSrgb()) {
              return FormatInfo{.texFormat = DXGI_FORMAT_BC3_TYPELESS, .srvFormat = DXGI_FORMAT_BC3_UNORM_SRGB, .compressedData = std::move(blockCompressedData)};
            }
            return FormatInfo{.texFormat = DXGI_FORMAT_BC3_TYPELESS, .srvFormat = DXGI_FORMAT_BC3_UNORM, .compressedData = std::move(blockCompressedData)};
          }
        }
      }
      if (channelCount == 1) {
        return FormatInfo{.texFormat = DXGI_FORMAT_R8_TYPELESS, .srvFormat = DXGI_FORMAT_R8_UNORM};
      }
      if (channelCount == 2) {
        return FormatInfo{.texFormat = DXGI_FORMAT_R8G8_TYPELESS, .srvFormat = DXGI_FORMAT_R8G8_UNORM};
      }
      if (channelCount == 3 || channelCount == 4) {
        if (channelCount == 3) {
          mImgData->AppendChannel(255);
          mChannelCount = 4;
        }
        if (mImgData->IsSrgb()) {
          return FormatInfo{.texFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS, .srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB};
        }
        return FormatInfo{.texFormat = DXGI_FORMAT_R8G8B8A8_TYPELESS, .srvFormat = DXGI_FORMAT_R8G8B8A8_UNORM};
      }
      return FormatInfo{.texFormat = DXGI_FORMAT_UNKNOWN, .srvFormat = DXGI_FORMAT_UNKNOWN};
    }()
  };

  D3D11_TEXTURE2D_DESC const texDesc{
    .Width = clamp_cast<UINT>(mImgData->GetWidth()),
    .Height = clamp_cast<UINT>(mImgData->GetHeight()),
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = texFormat,
    .SampleDesc = {.Count = 1, .Quality = 0},
    .Usage = D3D11_USAGE_DEFAULT,
    .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };

  D3D11_SUBRESOURCE_DATA const subResData{
    .pSysMem = alternativeData ? alternativeData->bytes.get() : mImgData->GetData().data(),
    .SysMemPitch = static_cast<UINT>(alternativeData ? alternativeData->rowByteCount : mImgData->GetWidth() * mImgData->GetChannelCount())
  };

  if (FAILED(gRenderer.GetDevice()->CreateTexture2D(&texDesc, &subResData, mTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create GPU texture."};
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC const srvDesc{
    .Format = srvFormat,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D = {.MostDetailedMip = 0, .MipLevels = static_cast<UINT>(-1)}
  };

  if (FAILED(gRenderer.GetDevice()->CreateShaderResourceView(mTex.Get(), &srvDesc, mSrv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{"Failed to create GPU SRV."};
  }
}


Texture2D::Texture2D(Image img, bool const keepDataInCpuMemory, bool const allowBlockCompression) {
  SetImageData(std::move(img));
  Update(keepDataInCpuMemory, allowBlockCompression);
}


auto Texture2D::GetImageData() const noexcept -> ObserverPtr<Image const> {
  return mImgData ? mImgData.get() : nullptr;
}


auto Texture2D::SetImageData(Image img) noexcept -> void {
  if (!mImgData) {
    mImgData = std::make_unique<Image>(std::move(img));
  } else {
    *mImgData = std::move(img);
  }

  mWidth = mImgData->GetWidth();
  mHeight = mImgData->GetHeight();
  mChannelCount = mImgData->GetChannelCount();
}


auto Texture2D::GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView> {
  return mSrv.Get();
}


auto Texture2D::Update(bool const keepDataInCpuMemory, bool const allowBlockCompression) noexcept -> void {
  if (mImgData) {
    UploadToGpu(allowBlockCompression);

    if (!keepDataInCpuMemory) {
      ReleaseCpuMemory();
    }
  }
}


auto Texture2D::HasCpuMemory() const noexcept -> bool {
  return static_cast<bool>(mImgData);
}


auto Texture2D::ReleaseCpuMemory() -> void {
  if (mImgData) {
    mImgData.reset();
  }
}


auto Texture2D::GetWidth() const noexcept -> int {
  return mWidth;
}


auto Texture2D::GetHeight() const noexcept -> int {
  return mHeight;
}


auto Texture2D::GetChannelCount() const noexcept -> int {
  return mChannelCount;
}


auto Texture2D::OnDrawProperties(bool& changed) -> void {
  Resource::OnDrawProperties(changed);

  if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("%s", "Width");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetWidth()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Height");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetHeight()).c_str());

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Channel Count");

    ImGui::TableNextColumn();
    ImGui::Text("%s", std::to_string(GetChannelCount()).c_str());

    ImGui::EndTable();
  }

  auto const contentRegion{ImGui::GetContentRegionAvail()};
  auto const imgWidth{static_cast<float>(GetWidth())};
  auto const imgHeight{static_cast<float>(GetHeight())};
  auto const widthRatio{contentRegion.x / imgWidth};
  auto const heightRatio{contentRegion.y / imgHeight};
  ImVec2 displaySize;

  if (widthRatio > heightRatio) {
    displaySize.x = imgWidth * heightRatio;
    displaySize.y = imgHeight * heightRatio;
  } else {
    displaySize.x = imgWidth * widthRatio;
    displaySize.y = imgHeight * widthRatio;
  }

  ImGui::Image(GetSrv(), displaySize);
}
}
