#pragma once

#include <cstdint>

#include "wand/buffer_view.hpp"
#include "wand/sampler.hpp"

#include "sorcery/observer_ptr.hpp"


namespace sorcery::rendering {
struct ViewContext {
  std::uint32_t width;
  std::uint32_t height;

  wand::Sampler samp_point_clamp;
  wand::Sampler samp_point_wrap;

  ObserverPtr<wand::BufferView> per_view_cb;
  ObserverPtr<wand::BufferView> per_frame_cb;
};
}
