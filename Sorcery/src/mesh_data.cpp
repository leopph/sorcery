#include "mesh_data.hpp"

#include "rendering/shaders/shader_interop.h"


namespace sorcery {
std::uint16_t const kMeshletMaxVerts{MESHLET_MAX_VERTS};
std::uint16_t const kMeshletMaxPrims{MESHLET_MAX_PRIMS};
static_assert(kMeshletMaxVerts < kMeshletMaxPrims, "Meshlet max vertex count must be less than max primitive count!");
}
