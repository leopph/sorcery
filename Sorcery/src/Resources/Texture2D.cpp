#include "Texture2D.hpp"

#include <imgui.h>

#include "../Util.hpp"
#include "../Renderer.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{ "Texture2D" };
}


namespace sorcery {
auto Texture2D::UploadToGPU() -> void {
  D3D11_TEXTURE2D_DESC const texDesc{
    .Width = clamp_cast<UINT>(mImgData->get_width()),
    .Height = clamp_cast<UINT>(mImgData->get_height()),
    .MipLevels = 1,
    .ArraySize = 1,
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .SampleDesc = { .Count = 1, .Quality = 0 },
    .Usage = D3D11_USAGE_IMMUTABLE,
    .BindFlags = D3D11_BIND_SHADER_RESOURCE,
    .CPUAccessFlags = 0,
    .MiscFlags = 0
  };
  D3D11_SUBRESOURCE_DATA const texData{
    .pSysMem = mImgData->get_data().data(),
    .SysMemPitch = mImgData->get_width() * mImgData->get_num_channels()
  };

  if (FAILED(gRenderer.GetDevice()->CreateTexture2D(&texDesc, &texData, mTex.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create GPU texture." };
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC constexpr srvDesc{
    .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
    .ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D,
    .Texture2D = {
      .MostDetailedMip = 0,
      .MipLevels = static_cast<UINT>(-1)
    }
  };

  if (FAILED(gRenderer.GetDevice()->CreateShaderResourceView(mTex.Get(), &srvDesc, mSrv.ReleaseAndGetAddressOf()))) {
    throw std::runtime_error{ "Failed to create GPU SRV." };
  }
}


Texture2D::Texture2D(Image img, bool const keepDataInCPUMemory) {
  SetImageData(std::move(img));
  Update(keepDataInCPUMemory);
}


auto Texture2D::GetImageData() const noexcept -> ObserverPtr<Image const> {
  return mImgData
           ? mImgData
           : nullptr;
}


auto Texture2D::SetImageData(Image img, bool const allocateCPUMemoryIfNeeded) noexcept -> void {
  if (!mImgData) {
    if (!allocateCPUMemoryIfNeeded) {
      return;
    }

    mImgData = new Image{ std::move(img) };
  } else {
    *mImgData = std::move(img);
  }

  mWidth = mImgData->get_width();
  mHeight = mImgData->get_height();
  mChannelCount = mImgData->get_num_channels();
}


auto Texture2D::GetSrv() const noexcept -> ObserverPtr<ID3D11ShaderResourceView> {
  return mSrv.Get();
}


auto Texture2D::Update(bool const keepDataInCPUMemory) noexcept -> void {
  if (mImgData) {
    UploadToGPU();

    if (!keepDataInCPUMemory) {
      ReleaseCPUMemory();
    }
  }
}


auto Texture2D::ReleaseCPUMemory() -> void {
  if (mImgData) {
    delete mImgData;
    mImgData = nullptr;
  }
}


auto Texture2D::HasCPUMemory() const noexcept -> bool {
  return mImgData;
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

  auto const contentRegion{ ImGui::GetContentRegionAvail() };
  auto const imgWidth{ static_cast<float>(GetWidth()) };
  auto const imgHeight{ static_cast<float>(GetHeight()) };
  auto const widthRatio{ contentRegion.x / imgWidth };
  auto const heightRatio{ contentRegion.y / imgHeight };
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
