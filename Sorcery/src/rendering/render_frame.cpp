#include "render_frame.hpp"

#include <cassert>

#include "config.hpp"
#include "../Util.hpp"


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


auto RenderFrame::UploadBuffer(wand::SharedDeviceChildHandle<wand::Buffer> const& buf, UINT64 const byte_offset,
                               std::span<std::byte const> const data) -> void {
  assert(active_);
  assert(!submitted_);
  assert(!uploads_recorded_);
  uploader_.UploadBuffer(buf, byte_offset, data);
}


auto RenderFrame::RecordUploads() -> void {
  assert(active_);
  assert(!submitted_);
  assert(!uploads_recorded_);
  assert(queued_cmd_lists_.empty());

  uploads_recorded_ = true;

  if (uploader_.IsEmpty()) {
    return;
  }

  auto& cmd = AcquireCommandList();
  cmd.Begin(nullptr);
  uploader_.Record(cmd);
  cmd.End();
  EnqueueCommandList(cmd);
}


RenderFrame::RenderFrame(wand::GraphicsDevice& device, std::uint32_t const idx, std::uint32_t const prev_idx) :
  free_cmd_lists_{device},
  uploader_{device},
  idx_{idx},
  prev_idx_{prev_idx} {}


auto RenderFrame::ResetForReuse(std::uint64_t const frame_num) -> void {
  uploader_.Reset();
  free_cmd_lists_.Reset();
  queued_cmd_lists_.clear();

  num_ = frame_num;

  active_ = true;
  submitted_ = false;
  uploads_recorded_ = false;
}


auto RenderFrame::GetQueuedCommandLists() const -> std::span<ObserverPtr<wand::CommandList> const> {
  return queued_cmd_lists_;
}
}
