#include "sorcery/host/rendering/render_frame.hpp"

#include <cassert>


namespace sorcery::rendering {
auto RenderFrame::GetIndex() const -> std::uint32_t {
  return idx_;
}


auto RenderFrame::GetPreviousIndex() const -> std::uint32_t {
  return prev_idx_;
}


auto RenderFrame::GetNumber() const -> std::uint64_t {
  return num_;
}


auto RenderFrame::AcquireCommandList() -> wand::CommandList& {
  assert(active_);
  return free_cmd_lists_.Acquire();
}


auto RenderFrame::EnqueueCommandList(wand::CommandList& cmd) -> void {
  assert(active_);
  assert(!submitted_);
  queued_cmd_lists_.emplace_back(&cmd);
}


auto RenderFrame::GetUploadArena() -> UploadArena& {
  return upload_arena_;
}


RenderFrame::RenderFrame(wand::GraphicsDevice& device, std::uint32_t const idx, std::uint32_t const prev_idx) :
  free_cmd_lists_{device},
  upload_arena_{device},
  idx_{idx},
  prev_idx_{prev_idx} {}


auto RenderFrame::ResetForReuse(std::uint64_t const frame_num) -> void {
  upload_arena_.Reset();
  free_cmd_lists_.Reset();
  queued_cmd_lists_.clear();

  num_ = frame_num;

  active_ = true;
  submitted_ = false;
}


auto RenderFrame::GetQueuedCommandLists() const -> std::span<ObserverPtr<wand::CommandList> const> {
  return queued_cmd_lists_;
}
}
