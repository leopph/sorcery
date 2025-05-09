#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "graphics.hpp"
#include "../Core.hpp"


namespace sorcery::rendering {
class RenderTarget {
public:
  struct Desc {
    UINT width{1024};
    UINT height{1024};

    std::optional<DXGI_FORMAT> color_format{DXGI_FORMAT_R8G8B8A8_UNORM};
    std::optional<DXGI_FORMAT> depth_stencil_format{DXGI_FORMAT_D32_FLOAT};

    UINT sample_count{1};

    std::wstring debug_name;

    bool enable_unordered_access{false};

    std::array<float, 4> color_clear_value{0.0f, 0.0f, 0.0f, 1.0f};
    float depth_clear_value{0.0f};
    std::uint8_t stencil_clear_value{0};

    graphics::TextureDimension dimension{graphics::TextureDimension::k2D};
    UINT16 depth_or_array_size{1};

    LEOPPHAPI [[nodiscard]] auto operator==(Desc const& other) const -> bool;
  };


  [[nodiscard]] LEOPPHAPI static auto New(graphics::GraphicsDevice& device,
                                          Desc const& desc) -> std::unique_ptr<RenderTarget>;

  RenderTarget(RenderTarget const&) = delete;
  RenderTarget(RenderTarget&&) = delete;

  ~RenderTarget() = default;

  auto operator=(RenderTarget const&) -> void = delete;
  auto operator=(RenderTarget&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetDesc() const noexcept -> Desc const&;
  [[nodiscard]] LEOPPHAPI auto
  GetColorTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;
  [[nodiscard]] LEOPPHAPI auto
  GetDepthStencilTex() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Texture> const&;

private:
  RenderTarget(Desc desc, graphics::SharedDeviceChildHandle<graphics::Texture> color_tex,
               graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex);

  Desc desc_;

  graphics::SharedDeviceChildHandle<graphics::Texture> color_tex_;
  graphics::SharedDeviceChildHandle<graphics::Texture> depth_stencil_tex_;
};
}
