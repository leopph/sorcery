#pragma once

#include <array>

#include "shaders/ShaderInterop.h"


namespace sorcery {
struct ShadowCascadeBoundary {
  float nearClip;
  float farClip;
};


using ShadowCascadeBoundaries = std::array<ShadowCascadeBoundary, MAX_CASCADE_COUNT>;
}
