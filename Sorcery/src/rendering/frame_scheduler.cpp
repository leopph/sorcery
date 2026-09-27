#include "frame_scheduler.hpp"

#include <cassert>


namespace sorcery::rendering {
auto FrameScheduler::AcquireFrame() -> RenderFrame& {
  auto& frame = *frames_[next_frame_idx_];

  if (frame.fence_competion_val_ != 0) {
    frame_fence_->Wait(frame.fence_competion_val_);
  }

  frame.ResetForReuse(next_frame_num_++);

  next_frame_idx_ = (next_frame_idx_ + 1) % kFramesInFlight;
  return frame;
}


auto FrameScheduler::SubmitFrame(RenderFrame& frame) -> void {
  assert(frame.active_);
  assert(!frame.submitted_);

  for (auto const cmd : frame.GetQueuedCommandLists()) {
    device_->ExecuteCommandLists(std::span{cmd.Get(), 1});
  }

  auto const completion_val{frame_fence_->GetNextValue()};

  device_->SignalFence(*frame_fence_);

  frame.fence_competion_val_ = completion_val;
  frame.submitted_ = true;
  frame.active_ = false;
}


auto FrameScheduler::GetFrameNumber() const -> std::uint64_t {
  return next_frame_num_;
}


FrameScheduler::FrameScheduler(wand::GraphicsDevice& device) :
  device_{&device},
  frame_fence_{device.CreateFence(0)} {
  for (auto i = 0u; i < kFramesInFlight; i++) {
    frames_[i].reset(new RenderFrame{device, i, (i + kFramesInFlight - 1) % kFramesInFlight});
  }
}


FrameScheduler::~FrameScheduler() {
  device_->WaitIdle();
}
}
