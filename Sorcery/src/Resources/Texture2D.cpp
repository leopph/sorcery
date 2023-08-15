#include "Texture2D.hpp"

#include "../Util.hpp"
#include "../Renderer.hpp"

#include <imgui.h>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{"Texture2D"};
}


namespace sorcery {
Texture2D::Texture2D(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept :
  mTex{&tex},
  mSrv{&srv} {
  mTex->AddRef();
  mSrv->AddRef();

  D3D11_TEXTURE2D_DESC desc;
  mTex->GetDesc(&desc);

  mWidth = static_cast<int>(desc.Width);
  mHeight = static_cast<int>(desc.Height);
}


auto Texture2D::GetSrv() const noexcept -> NotNull<ObserverPtr<ID3D11ShaderResourceView>> {
  return mSrv.Get();
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
