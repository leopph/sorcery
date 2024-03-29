#pragma once

#include <array>

#include "shaders\shader_interop.h"


namespace sorcery::rendering {
struct ShadowCascadeBoundary {
  float nearClip;
  float farClip;
};


using ShadowCascadeBoundaries = std::array<ShadowCascadeBoundary, MAX_CASCADE_COUNT>;
}
