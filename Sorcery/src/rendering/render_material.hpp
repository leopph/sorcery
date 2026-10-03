#pragma once

#include <cstdint>

#include "wand/wand.hpp"


namespace sorcery::rendering {
class RenderMaterial {
public:
  explicit RenderMaterial(wand::GraphicsDevice& device);
  RenderMaterial(RenderMaterial const& other) = delete;
  RenderMaterial(RenderMaterial&& other) noexcept = delete;

  ~RenderMaterial() = default;

  auto operator=(RenderMaterial const&) -> RenderMaterial& = delete;
  auto operator=(RenderMaterial&&) noexcept -> RenderMaterial& = delete;

  [[nodiscard]] auto GetBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]] auto GetRevision() const -> std::uint64_t;
  auto SetRevision(std::uint64_t rev) -> void;

private:
  wand::SharedDeviceChildHandle<wand::Buffer> cb_;
  std::uint64_t revision_{0};
};
}
