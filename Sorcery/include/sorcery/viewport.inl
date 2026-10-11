#pragma once

namespace sorcery {
[[nodiscard]] constexpr auto GetWidth(NormalizedViewport const& viewport) noexcept -> float {
  return viewport.right - viewport.left;
}


[[nodiscard]] constexpr auto GetHeight(NormalizedViewport const& viewport) noexcept -> float {
  return viewport.bottom - viewport.top;
}
}
