#include "temporary_render_target_pool.hpp"

#include <algorithm>
#include <utility>


namespace sorcery::rendering {
TemporaryRenderTargetPool::TemporaryRenderTargetPool(wand::GraphicsDevice& device) :
  device_{&device} {}


auto TemporaryRenderTargetPool::Acquire(
  RenderTarget::Desc const& desc,
  RenderFrame const& frame
) -> ObserverPtr<RenderTarget> {
  auto const frame_num = frame.GetNumber();

  for (auto& [rt, last_used_frame] : rts_) {
    if (last_used_frame + kFramesInFlight - 1 < frame_num && rt->GetDesc() == desc) {
      last_used_frame = frame_num;
      return MakeObserver(rt.get());
    }
  }

  auto rt = RenderTarget::New(*device_, desc);

  if (!rt) {
    return nullptr;
  }

  return MakeObserver(rts_.emplace_back(std::move(rt), frame_num).rt.get());
}


auto TemporaryRenderTargetPool::CollectGarbage(RenderFrame const& frame) -> void {
  auto const frame_num = frame.GetNumber();
  auto const [first, last] = std::ranges::remove_if(rts_, [frame_num](Record const& record) {
    return record.last_used_frame < frame_num && frame_num - record.last_used_frame >= kRtGcAge;
  });
  rts_.erase(first, last);
}
}
