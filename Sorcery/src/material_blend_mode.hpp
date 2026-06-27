#pragma once

#include <cstdint>

#include "rendering/shaders/shader_interop.h"


namespace sorcery {
enum class MaterialBlendMode : std::uint32_t {
  kOpaque    = BLEND_MODE_OPAQUE,
  kAlphaClip = BLEND_MODE_ALPHA_CLIP
};
}
