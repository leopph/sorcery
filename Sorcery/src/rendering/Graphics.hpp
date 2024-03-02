#pragma once

namespace sorcery {
class Graphics {
  constexpr static bool IS_DEPTH_BUFFER_REVERSED{true};

public:
  [[nodiscard]] consteval static auto IsUsingReversedZ() noexcept -> bool;
  [[nodiscard]] consteval static auto GetDepthClearValueForRendering() noexcept -> float;
  inline static auto GetProjectionMatrixForRendering(Matrix4& projMtx) noexcept -> Matrix4&;
  inline static auto GetProjectionMatrixForRendering(Matrix4&& projMtx) noexcept -> Matrix4&&;
};


inline auto Graphics::GetProjectionMatrixForRendering(Matrix4& projMtx) noexcept -> Matrix4& {
  if constexpr (IS_DEPTH_BUFFER_REVERSED) {
    projMtx *= Matrix4{
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, -1, 0,
      0, 0, 1, 1
    };
  }

  return projMtx;
}


inline auto Graphics::GetProjectionMatrixForRendering(Matrix4&& projMtx) noexcept -> Matrix4&& {
  return std::move(GetProjectionMatrixForRendering(projMtx));
}


consteval auto Graphics::IsUsingReversedZ() noexcept -> bool {
  return IS_DEPTH_BUFFER_REVERSED;
}


consteval auto Graphics::GetDepthClearValueForRendering() noexcept -> float {
  if constexpr (IS_DEPTH_BUFFER_REVERSED) {
    return 0.0f;
  } else {
    return 1.0f;
  }
}
}
