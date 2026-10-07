#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "../Core.hpp"
#include "wand/wand.hpp"


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

    wand::TextureDimension dimension{wand::TextureDimension::k2D};
    UINT16 depth_or_array_size{1};

    LEOPPHAPI [[nodiscard]] auto operator==(Desc const& other) const -> bool;
  };


  [[nodiscard]] LEOPPHAPI static auto New(wand::GraphicsDevice& device,
                                          Desc const& desc) -> std::unique_ptr<RenderTarget>;

  RenderTarget(RenderTarget const&) = delete;
  RenderTarget(RenderTarget&&) = delete;

  ~RenderTarget() = default;

  auto operator=(RenderTarget const&) -> void = delete;
  auto operator=(RenderTarget&&) -> void = delete;

  [[nodiscard]] LEOPPHAPI auto GetDesc() const noexcept -> Desc const&;
  [[nodiscard]] LEOPPHAPI auto GetColorTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const&;
  [[nodiscard]] LEOPPHAPI auto GetDepthStencilTex() const noexcept -> wand::SharedDeviceHandle<wand::Texture> const&;

private:
  RenderTarget(Desc desc, wand::SharedDeviceHandle<wand::Texture> color_tex,
               wand::SharedDeviceHandle<wand::Texture> depth_stencil_tex);

  Desc desc_;

  wand::SharedDeviceHandle<wand::Texture> color_tex_;
  wand::SharedDeviceHandle<wand::Texture> depth_stencil_tex_;
};
}
