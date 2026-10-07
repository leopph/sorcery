#pragma once

#include <array>
#include <cstdint>
#include <optional>

#include "config.hpp"
#include "render_mesh_instance_state.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class SkinnedRenderMeshInstance {
public:
  auto Init(wand::GraphicsDevice& device, std::uint64_t vtx_count, std::uint64_t bone_count) -> void;

  [[nodiscard]]
  auto GetState() -> RenderMeshInstanceState&;
  [[nodiscard]]
  auto GetState() const -> RenderMeshInstanceState const&;

  [[nodiscard]]
  auto GetSkinnedPositionBuffer(unsigned frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView>;
  [[nodiscard]]
  auto GetSkinnedNormalBuffer(unsigned frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView>;
  [[nodiscard]]
  auto GetSkinnedTangentBuffer(unsigned frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView>;
  [[nodiscard]]
  auto GetBoneMatrixBuffer(unsigned frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView>;

  [[nodiscard]]
  auto GetVertexCapacity() const -> std::uint64_t;
  [[nodiscard]]
  auto GetBoneCapacity() const -> std::uint64_t;

  [[nodiscard]]
  auto GetLastSkinningFrame() const -> std::optional<std::uint64_t>;
  auto SetLastSkinningFrame(std::uint64_t frame) -> void;
  auto InvalidateSkinningHistory() -> void;

private:
  RenderMeshInstanceState state_;

  std::array<wand::SharedDeviceHandle<wand::BufferView>, kFramesInFlight> skinned_pos_buf_;
  std::array<wand::SharedDeviceHandle<wand::BufferView>, kFramesInFlight> skinned_norm_buf_;
  std::array<wand::SharedDeviceHandle<wand::BufferView>, kFramesInFlight> skinned_tan_buf_;
  std::array<wand::SharedDeviceHandle<wand::BufferView>, kFramesInFlight> bone_mtx_buf_;

  std::uint64_t vtx_capacity_{};
  std::uint64_t bone_capacity_{};
  std::optional<std::uint64_t> last_skinning_frame_{};
};
}
