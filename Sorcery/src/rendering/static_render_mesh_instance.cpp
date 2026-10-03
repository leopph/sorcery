#include "static_render_mesh_instance.hpp"


namespace sorcery::rendering {
auto StaticRenderMeshInstance::GetState() -> RenderMeshInstanceState& {
  return state_;
}


auto StaticRenderMeshInstance::GetState() const -> RenderMeshInstanceState const& {
  return state_;
}
}
