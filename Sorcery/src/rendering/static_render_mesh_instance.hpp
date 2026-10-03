#pragma once

#include "render_mesh_instance_state.hpp"


namespace sorcery::rendering {
class StaticRenderMeshInstance {
public:
  [[nodiscard]]
  auto GetState() -> RenderMeshInstanceState&;
  [[nodiscard]]
  auto GetState() const -> RenderMeshInstanceState const&;

private:
  RenderMeshInstanceState state_;
};
}
