#pragma once

#include "Resource.hpp"
#include "../Image.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Cubemap final : public Resource {
  RTTR_ENABLE(Resource)

  graphics::UniqueHandle<graphics::Texture> tex_;

public:
  LEOPPHAPI Cubemap(ID3D11Texture2D& tex, ID3D11ShaderResourceView& srv) noexcept;

  [[nodiscard]] LEOPPHAPI auto GetTex() const noexcept -> graphics::Texture*;
};
}
