#pragma once

#include <array>
#include <cstdint>

#include "wand/wand.hpp"

#include "sorcery/object_id.hpp"


namespace sorcery::rendering {
class RenderMaterial {
public:
  explicit RenderMaterial(wand::GraphicsDevice& device);
  RenderMaterial(RenderMaterial const& other) = delete;
  RenderMaterial(RenderMaterial&& other) noexcept = delete;

  ~RenderMaterial() = default;

  auto operator=(RenderMaterial const&) -> RenderMaterial& = delete;
  auto operator=(RenderMaterial&&) noexcept -> RenderMaterial& = delete;

  [[nodiscard]]
  auto GetBufferView() const -> wand::SharedDeviceHandle<wand::BufferView> const&;

  [[nodiscard]]
  auto GetRevision() const -> std::uint64_t;
  auto SetRevision(std::uint64_t rev) -> void;

  [[nodiscard]]
  auto GetTextureIds() const -> std::array<ObjectId, 7> const&;
  auto SetTextureIds(std::array<ObjectId, 7> const& ids) -> void;

private:
  wand::SharedDeviceHandle<wand::BufferView> cb_;
  std::uint64_t revision_{0};
  std::array<ObjectId, 7> tex_ids_;
};
}
