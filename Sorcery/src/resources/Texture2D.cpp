#include "Texture2D.hpp"

#include <utility>

#include <DirectXTex.h>

#include "../app.hpp"
#include "../rendering/render_manager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Texture2D>{"Texture2D"};
}


namespace sorcery {
Texture2D::Texture2D(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept :
  tex_{std::move(tex)} {
  auto const desc{tex_->GetDesc()};
  m_width_ = static_cast<int>(desc.width);
  m_height_ = static_cast<int>(desc.height);
  m_channel_count_ =
    DirectX::IsCompressed(desc.format)
      ? [&desc] {
        switch (DirectX::MakeTypeless(desc.format)) {
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
      }()
      : static_cast<unsigned>(DirectX::BitsPerPixel(desc.format) / DirectX::BitsPerColor(desc.format));
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
}
