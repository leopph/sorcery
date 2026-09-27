#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "config.hpp"
#include "render_frame.hpp"
#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class FrameScheduler {
public:
  [[nodiscard]]
  auto AcquireFrame() -> RenderFrame&;
  auto SubmitFrame(RenderFrame& frame) -> void;

  [[nodiscard]]
  auto GetFrameNumber() const -> std::uint64_t;

  explicit FrameScheduler(wand::GraphicsDevice& device);
  FrameScheduler(FrameScheduler const& other) = delete;
  FrameScheduler(FrameScheduler&& other) noexcept = delete;

  ~FrameScheduler();

  auto operator=(FrameScheduler const& other) -> FrameScheduler& = delete;
  auto operator=(FrameScheduler&& other) noexcept -> FrameScheduler& = delete;

private:
  ObserverPtr<wand::GraphicsDevice> device_;
  wand::SharedDeviceChildHandle<wand::Fence> frame_fence_;
  std::array<std::unique_ptr<RenderFrame>, kFramesInFlight> frames_;

  std::uint32_t next_frame_idx_{0};
  std::uint64_t next_frame_num_{0};
};
}
