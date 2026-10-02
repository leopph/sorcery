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
  auto GetPositionBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetNormalBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetTangentBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetUvBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetBoneWeightBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetBoneIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetMeshletBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetVertexIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetPrimitiveIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;
  [[nodiscard]]
  auto GetCullDataBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const&;

  [[nodiscard]]
  auto GetRevision() const -> std::uint64_t;
  auto SetRevision(std::uint64_t rev) -> void;

private:
  wand::SharedDeviceChildHandle<wand::Buffer> pos_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> norm_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> tan_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> uv_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> bone_weight_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> bone_idx_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> meshlet_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> vertex_idx_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> prim_idx_buf_;
  wand::SharedDeviceChildHandle<wand::Buffer> cull_data_buf_;
  std::uint64_t revision_{};
};
}
