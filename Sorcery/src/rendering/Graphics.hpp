#pragma once

namespace sorcery::graphics {
inline auto GetProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4 {
  return proj_mtx * Matrix4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 1};
}
}
