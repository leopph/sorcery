#pragma once

#include "Resource.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Texture2D final : public Resource {
  RTTR_ENABLE(Resource)
  graphics::SharedDeviceChildHandle<graphics::Texture> tex_;

  unsigned m_width_;
  unsigned m_height_;
  unsigned m_channel_count_;

public:
  LEOPPHAPI explicit Texture2D(graphics::SharedDeviceChildHandle<graphics::Texture> tex) noexcept;
  Texture2D(Texture2D const&) = delete;
  Texture2D(Texture2D&&) noexcept = default;

  LEOPPHAPI ~Texture2D() override;

  auto operator=(Texture2D const&) -> void = delete;
  auto operator=(Texture2D&&) noexcept -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetTex() const -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> unsigned;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> unsigned;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> unsigned;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
