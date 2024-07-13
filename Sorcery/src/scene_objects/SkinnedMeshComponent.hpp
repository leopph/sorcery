#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>

#include "MeshComponentBase.hpp"
#include "../rendering/graphics.hpp"
#include "../rendering/render_manager.hpp"


namespace sorcery {
class SkinnedMeshComponent final : public MeshComponentBase {
  RTTR_ENABLE(MeshComponentBase)

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;
  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  LEOPPHAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;

  LEOPPHAPI auto SetMesh(Mesh* mesh) noexcept -> void override;

  LEOPPHAPI auto Start() -> void override;
  LEOPPHAPI auto Update() -> void override;

  LEOPPHAPI SkinnedMeshComponent();

  [[nodiscard]] LEOPPHAPI auto GetSkinnedVertexBuffers() const noexcept -> std::span<
    graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()>;
  [[nodiscard]] LEOPPHAPI auto GetSkinnedNormalBuffers() const noexcept -> std::span<
    graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()>;
  [[nodiscard]] LEOPPHAPI auto GetSkinnedTangentBuffers() const noexcept -> std::span<
    graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()>;
  [[nodiscard]] LEOPPHAPI auto GetBoneMatrixBuffers() const noexcept -> std::span<
    graphics::SharedDeviceChildHandle<graphics::Buffer> const, rendering::RenderManager::GetMaxFramesInFlight()>;

  [[nodiscard]] LEOPPHAPI auto GetCurrentAnimation() const -> std::optional<Animation>;
  [[nodiscard]] LEOPPHAPI auto GetCurrentAnimationTime() const noexcept -> float;

private:
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  skinned_vertex_buffers_;
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  skinned_normal_buffers_;
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  skinned_tangent_buffers_;
  std::array<graphics::SharedDeviceChildHandle<graphics::Buffer>, rendering::RenderManager::GetMaxFramesInFlight()>
  bone_matrix_buffers_;

  std::optional<std::size_t> cur_animation_idx_;
  float cur_animation_time_ticks_{0};
  float cur_anim_delta_time_{0};
};
}
