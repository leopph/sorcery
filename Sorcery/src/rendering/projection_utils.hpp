#pragma once

#include "sorcery/Math.hpp"


namespace sorcery::rendering {
auto TransformProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4;
}
