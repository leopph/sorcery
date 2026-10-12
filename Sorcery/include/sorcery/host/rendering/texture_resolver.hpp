#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXTex.h>

#include <wand/wand.hpp>

#include <sorcery/Core.hpp>
#include <sorcery/observer_ptr.hpp>


namespace sorcery {
class Texture2D;
class Cubemap;


namespace rendering {
class RenderResourceRegistry;
class RenderTexture;
class UploadArena;


class TextureResolver {
public:
  TextureResolver(wand::GraphicsDevice& device, RenderResourceRegistry& registry);
  TextureResolver(TextureResolver const& other) = delete;
  TextureResolver(TextureResolver&& other) noexcept = delete;

  ~TextureResolver() = default;

  auto operator=(TextureResolver const& other) -> TextureResolver& = delete;
  auto operator=(TextureResolver&& other) noexcept -> TextureResolver& = delete;

  [[nodiscard]] SORCERYAPI
  auto Resolve(
    Texture2D& tex,
    UploadArena& upload_arena,
    wand::CommandList& cmd
  ) const -> wand::SharedDeviceHandle<wand::Texture>;

  [[nodiscard]] SORCERYAPI
  auto Resolve(
    Cubemap& cubemap,
    UploadArena& upload_arena,
    wand::CommandList& cmd
  ) const -> wand::SharedDeviceHandle<wand::Texture>;

private:
  auto Upload(
    DirectX::ScratchImage const& img,
    RenderTexture& render_tex,
    UploadArena& upload_arena,
    wand::CommandList& cmd
  ) const -> void;

  ObserverPtr<wand::GraphicsDevice> device_;
  ObserverPtr<RenderResourceRegistry> registry_;
};
}
}
