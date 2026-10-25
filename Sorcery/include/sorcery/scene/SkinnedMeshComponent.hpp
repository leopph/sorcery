#pragma once

#include <cstddef>
#include <optional>

#include <sorcery/scene/MeshComponentBase.hpp>


namespace sorcery {
class SkinnedMeshComponent final : public MeshComponentBase {
  RTTR_ENABLE(MeshComponentBase)

public:
  SORCERYAPI
  auto OnDrawGizmosSelected() -> void override;

  [[nodiscard]] SORCERYAPI
  auto Clone() -> std::unique_ptr<SceneObject> override;

  SORCERYAPI
  auto OnAfterEnteringWorld(World& world) -> void override;
  SORCERYAPI
  auto OnBeforeExitingWorld(World& world) -> void override;

  SORCERYAPI
  auto SetMesh(AssetRef<Mesh> mesh) noexcept -> void override;

  SORCERYAPI
  auto Start() -> void override;
  SORCERYAPI
  auto Update() -> void override;

  SORCERYAPI SkinnedMeshComponent();

  [[nodiscard]] SORCERYAPI
  auto GetCurrentAnimationIndex() const -> std::optional<std::size_t>;

  SORCERYAPI
  auto SetCurrentAnimationIndex(std::optional<size_t> idx) -> void;

  SORCERYAPI
  auto Play() -> void;
  SORCERYAPI
  auto Stop() -> void;
  [[nodiscard]]
  SORCERYAPI
  auto IsPlaying() const -> bool;

  [[nodiscard]] SORCERYAPI
  auto GetCurrentAnimation() const -> std::optional<Animation>;
  [[nodiscard]] SORCERYAPI
  auto GetCurrentAnimationTime() const noexcept -> float;

private:
  std::optional<std::size_t> cur_animation_idx_;
  float cur_animation_time_ticks_{0};
  float cur_anim_delta_time_{0};
  bool is_playing_{false};
};
}
