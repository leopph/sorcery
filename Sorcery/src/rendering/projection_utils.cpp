#include "projection_utils.hpp"

#include "shaders/shader_interop.h"


namespace sorcery::rendering {
auto TransformProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4 {
#ifdef REVERSE_Z
  return proj_mtx * Matrix4{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1, 0, 0, 0, 1, 1};
#else
  return proj_mtx;
#endif
}
}
