#pragma once

#include <cstdint>

#include "wand/wand.hpp"


namespace sorcery::rendering {
class RenderMesh {
public:
  auto Init(wand::GraphicsDevice& device, std::uint64_t vtx_count, std::uint64_t meshlet_count,
            std::uint64_t vtx_idx_byte_count, std::uint64_t prim_idx_count, std::uint64_t cull_data_count,
            bool has_skinning_data) -> void;

  [[nodiscard]]
  auto GetPositionBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetNormalBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetTangentBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetUvBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetBoneWeightBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetBoneIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetMeshletBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetVertexIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetPrimitiveIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;
  [[nodiscard]]
  auto GetCullDataBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const&;

  [[nodiscard]]
  auto GetRevision() const -> std::uint64_t;
  auto SetRevision(std::uint64_t rev) -> void;

private:
  wand::SharedDeviceHandle<wand::BufferView> pos_buf_;
  wand::SharedDeviceHandle<wand::BufferView> norm_buf_;
  wand::SharedDeviceHandle<wand::BufferView> tan_buf_;
  wand::SharedDeviceHandle<wand::BufferView> uv_buf_;
  wand::SharedDeviceHandle<wand::BufferView> bone_weight_buf_;
  wand::SharedDeviceHandle<wand::BufferView> bone_idx_buf_;
  wand::SharedDeviceHandle<wand::BufferView> meshlet_buf_;
  wand::SharedDeviceHandle<wand::BufferView> vertex_idx_buf_;
  wand::SharedDeviceHandle<wand::BufferView> prim_idx_buf_;
  wand::SharedDeviceHandle<wand::BufferView> cull_data_buf_;
  std::uint64_t revision_{};
};
}
