#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "config.hpp"
#include "render_frame.hpp"
#include "render_target.hpp"
#include "../observer_ptr.hpp"
#include "wand/graphics_device.hpp"


namespace sorcery::rendering {
class TemporaryRenderTargetPool {
public:
  SORCERYAPI explicit TemporaryRenderTargetPool(wand::GraphicsDevice& device);
  TemporaryRenderTargetPool(TemporaryRenderTargetPool const&) = delete;
  TemporaryRenderTargetPool(TemporaryRenderTargetPool&&) = delete;

  ~TemporaryRenderTargetPool() = default;

  auto operator=(TemporaryRenderTargetPool const&) -> void = delete;
  auto operator=(TemporaryRenderTargetPool&&) -> void = delete;

  // Acquires a temporary render target for the specified frame.
  // The returned observer must not be used in subsequent frames.
  // The pool guarantees the resource remains alive until all GPU
  // work from the acquiring frame has completed.
  [[nodiscard]] SORCERYAPI
  auto Acquire(RenderTarget::Desc const& desc, RenderFrame const& frame) -> ObserverPtr<RenderTarget>;

  SORCERYAPI
  auto CollectGarbage(RenderFrame const& frame) -> void;

private:
  struct Record {
    std::unique_ptr<RenderTarget> rt;
    std::uint64_t last_used_frame;
  };


  ObserverPtr<wand::GraphicsDevice> device_;
  std::vector<Record> rts_;

  static std::uint64_t constexpr kRtGcAge{10};
  static_assert(kRtGcAge >= kFramesInFlight && "Temporary RT lifetime too short!");
};
}
