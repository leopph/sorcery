#pragma once

namespace sorcery {
class Graphics {
public:
  inline static auto GetProjectionMatrixForRendering(Matrix4& proj_mtx) noexcept -> Matrix4&;
  inline static auto GetProjectionMatrixForRendering(Matrix4&& proj_mtx) noexcept -> Matrix4&&;
};


inline auto Graphics::GetProjectionMatrixForRendering(Matrix4& proj_mtx) noexcept -> Matrix4& {
  proj_mtx *= Matrix4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 1};

  return proj_mtx;
}


inline auto Graphics::GetProjectionMatrixForRendering(Matrix4&& proj_mtx) noexcept -> Matrix4&& {
  return std::move(GetProjectionMatrixForRendering(proj_mtx));
}
}
