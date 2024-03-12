#pragma once

#include "Resource.hpp"
#include "../Image.hpp"
#include "../Util.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Texture2D final : public Resource {
  RTTR_ENABLE(Resource)

  graphics::SharedDeviceChildHandle<graphics::Texture> tex_;

  int mWidth;
  int mHeight;
  int mChannelCount;

public:
  LEOPPHAPI Texture2D(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetTex() const -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;

  [[nodiscard]] LEOPPHAPI auto GetWidth() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetHeight() const noexcept -> int;
  [[nodiscard]] LEOPPHAPI auto GetChannelCount() const noexcept -> int;

  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
};
}
