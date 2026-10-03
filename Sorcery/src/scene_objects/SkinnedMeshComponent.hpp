#pragma once

#include <cstddef>
#include <optional>

#include "MeshComponentBase.hpp"


namespace sorcery {
class SkinnedMeshComponent final : public MeshComponentBase {
  RTTR_ENABLE(MeshComponentBase)

public:
  LEOPPHAPI auto OnDrawGizmosSelected() -> void override;
  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  LEOPPHAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;

  LEOPPHAPI auto SetMesh(ResourceRef<Mesh> mesh) noexcept -> void override;

  LEOPPHAPI auto Start() -> void override;
  LEOPPHAPI auto Update() -> void override;

  LEOPPHAPI SkinnedMeshComponent();

  [[nodiscard]] SORCERYAPI
  auto GetCurrentAnimationIndex() const -> std::optional<std::size_t>;

  SORCERYAPI
  auto SetCurrentAnimationIndex(std::optional<size_t> idx) -> void;

  [[nodiscard]] LEOPPHAPI auto GetCurrentAnimation() const -> std::optional<Animation>;
  [[nodiscard]] LEOPPHAPI auto GetCurrentAnimationTime() const noexcept -> float;

private:
  std::optional<std::size_t> cur_animation_idx_;
  float cur_animation_time_ticks_{0};
  float cur_anim_delta_time_{0};
};
}
