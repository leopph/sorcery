#pragma once

#include <cstdint>

#include "../Math.hpp"
#include "../object_id.hpp"


namespace sorcery::rendering {
struct RenderMeshInstanceState {
  ObjectId src_mesh_id{};
  std::uint64_t src_mesh_rev{};
  Matrix4 prev_frame_transform{Matrix4::Identity()};
};
}
