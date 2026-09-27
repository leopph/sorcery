#pragma once

#include <cstdint>

#include "command_list_pool.hpp"
#include "../Core.hpp"


namespace sorcery::rendering {
class FrameScheduler;


class RenderFrame {
public:
  [[nodiscard]] SORCERYAPI
  auto GetIndex() const -> std::uint32_t;

  [[nodiscard]] SORCERYAPI
  auto GetPreviousIndex() const -> std::uint32_t;

  [[nodiscard]] SORCERYAPI
  auto GetNumber() const -> std::uint64_t;

  [[nodiscard]] SORCERYAPI
  auto AcquireCommandList() -> wand::CommandList&;

  SORCERYAPI
  auto EnqueueCommandList(wand::CommandList& cmd) -> void;

  auto KeepAlive(wand::SharedDeviceChildHandle<wand::Buffer> buf) -> void;
  auto KeepAlive(wand::SharedDeviceChildHandle<wand::Texture> tex) -> void;

  RenderFrame(RenderFrame const& other) = delete;
  RenderFrame(RenderFrame&& other) noexcept = delete;

  ~RenderFrame() = default;

  auto operator=(RenderFrame const& other) -> RenderFrame& = delete;
  auto operator=(RenderFrame&& other) noexcept -> RenderFrame& = delete;

private:
  RenderFrame(wand::GraphicsDevice& device, std::uint32_t idx, std::uint32_t prev_idx);

  auto ResetForReuse(std::uint64_t frame_num) -> void;

  [[nodiscard]]
  auto GetQueuedCommandLists() const -> std::span<ObserverPtr<wand::CommandList> const>;

  CommandListPool free_cmd_lists_;
  std::vector<ObserverPtr<wand::CommandList>> queued_cmd_lists_;
  std::uint32_t idx_;
  std::uint32_t prev_idx_;
  std::uint64_t num_{0};
  UINT64 fence_competion_val_{0};
  bool active_{false};
  bool submitted_{false};

  friend class FrameScheduler;
};
}
