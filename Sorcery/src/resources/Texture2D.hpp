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

  [[nodiscard]] LEOPPHAPI auto GetTex() const -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> unsigned;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> unsigned;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> unsigned;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
