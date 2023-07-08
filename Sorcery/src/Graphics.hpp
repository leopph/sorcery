#pragma once

#include <algorithm>


namespace sorcery {
class Graphics {
  constexpr static bool IS_DEPTH_BUFFER_REVERSED{ true };

public:
  [[nodiscard]] consteval static auto IsDepthBufferReversed() -> bool;
  [[nodiscard]] consteval static auto GetDepthClearValueForReversedDepth() -> float;
  constexpr static auto AdjustClipPlanesForReversedDepth(float& nearClipPlane, float& farClipPlane) -> void;
};


consteval auto Graphics::IsDepthBufferReversed() -> bool {
  return IS_DEPTH_BUFFER_REVERSED;
}


consteval auto Graphics::GetDepthClearValueForReversedDepth() -> float {
  if constexpr (IS_DEPTH_BUFFER_REVERSED) {
    return 0.0f;
  } else {
    return 1.0f;
  }
}


constexpr auto Graphics::AdjustClipPlanesForReversedDepth(float& nearClipPlane, float& farClipPlane) -> void {
  if constexpr (IS_DEPTH_BUFFER_REVERSED) {
    std::swap(nearClipPlane, farClipPlane);
  }
}
}
