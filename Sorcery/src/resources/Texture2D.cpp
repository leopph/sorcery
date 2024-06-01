#include "Texture2D.hpp"

#include "../app.hpp"
#include "../rendering/render_manager.hpp"

#include <DirectXTex.h>
#include <imgui.h>

#include <format>
#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{"Texture2D"};
}


namespace sorcery {
Texture2D::Texture2D(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept :
  tex_{std::move(tex)} {
  auto const desc{tex_->GetDesc()};
  m_width_ = static_cast<int>(desc.width);
  m_height_ = static_cast<int>(desc.height);
  m_channel_count_ = static_cast<unsigned>(DirectX::BitsPerPixel(desc.format) / DirectX::BitsPerColor(desc.format));
}


Texture2D::~Texture2D() {
  App::Instance().GetRenderManager().KeepAliveWhileInUse(tex_);
}


auto Texture2D::GetTex() const -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return tex_;
}


auto Texture2D::GetWidth() const noexcept -> unsigned {
  return m_width_;
}


auto Texture2D::GetHeight() const noexcept -> unsigned {
  return m_height_;
}


auto Texture2D::GetChannelCount() const noexcept -> unsigned {
  return m_channel_count_;
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

  ImGui::Image(tex_.get(), displaySize);
}
}
