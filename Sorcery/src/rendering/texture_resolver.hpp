#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXTex.h>

#include "render_frame.hpp"
#include "render_resource_registry.hpp"
#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery {
class Texture2D;
class Cubemap;


namespace rendering {
class TextureResolver {
public:
  TextureResolver(wand::GraphicsDevice& device, RenderResourceRegistry& registry);
  TextureResolver(TextureResolver const& other) = delete;
  TextureResolver(TextureResolver&& other) noexcept = delete;

  ~TextureResolver() = default;

  auto operator=(TextureResolver const& other) -> TextureResolver& = delete;
  auto operator=(TextureResolver&& other) noexcept -> TextureResolver& = delete;

  [[nodiscard]] SORCERYAPI
  auto Resolve(Texture2D& tex, RenderFrame& frame) const -> wand::SharedDeviceHandle<wand::Texture>;

  [[nodiscard]] SORCERYAPI
  auto Resolve(Cubemap& cubemap, RenderFrame& frame) const -> wand::SharedDeviceHandle<wand::Texture>;

private:
  auto Upload(DirectX::ScratchImage const& img, RenderTexture& render_tex, RenderFrame& frame) const -> void;

  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<RenderResourceRegistry> registry_;
};
}
}
