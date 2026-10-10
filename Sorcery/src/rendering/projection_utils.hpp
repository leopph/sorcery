#pragma once

#include "../Math.hpp"


namespace sorcery::rendering {
auto TransformProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4;
}
