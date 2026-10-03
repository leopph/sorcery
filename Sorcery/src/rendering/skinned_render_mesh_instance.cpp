#include "skinned_render_mesh_instance.hpp"

#include <cassert>

#include "wand/flags.hpp"


namespace sorcery::rendering {
auto SkinnedRenderMeshInstance::Init(wand::GraphicsDevice& device, std::uint64_t const vtx_count,
                                     std::uint64_t const bone_count) -> void {
  vtx_capacity_ = vtx_count;
  bone_capacity_ = bone_count;

  for (auto i = 0u; i < kFramesInFlight; i++) {
    if (vtx_count != 0) {
      skinned_pos_buf_[i] = device.CreateBuffer(wand::BufferDesc{
          .size = vtx_count * sizeof(Vector4), .stride = sizeof(Vector4),
          .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kUnorderedAccess |
                   wand::BufferUsage::kCopyDestination
        },
        wand::CpuAccess::kNone);

      skinned_norm_buf_[i] = device.CreateBuffer(wand::BufferDesc{
          .size = vtx_count * sizeof(Vector4), .stride = sizeof(Vector4),
          .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kUnorderedAccess |
                   wand::BufferUsage::kCopyDestination
        },
        wand::CpuAccess::kNone);

      skinned_tan_buf_[i] = device.CreateBuffer(wand::BufferDesc{
          .size = vtx_count * sizeof(Vector4), .stride = sizeof(Vector4),
          .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kUnorderedAccess |
                   wand::BufferUsage::kCopyDestination
        },
        wand::CpuAccess::kNone);
    } else {
      skinned_pos_buf_[i].reset();
      skinned_norm_buf_[i].reset();
      skinned_tan_buf_[i].reset();
    }

    if (bone_count != 0) {
      bone_mtx_buf_[i] = device.CreateBuffer(wand::BufferDesc{
          .size = bone_count * sizeof(Matrix4), .stride = sizeof(Matrix4),
          .usage = wand::BufferUsage::kUnorderedAccess | wand::BufferUsage::kCopyDestination
        },
        wand::CpuAccess::kNone);
    } else {
      bone_mtx_buf_[i].reset();
    }
  }
}


auto SkinnedRenderMeshInstance::GetState() -> RenderMeshInstanceState& {
  return state_;
}


auto SkinnedRenderMeshInstance::GetState() const -> RenderMeshInstanceState const& {
  return state_;
}


auto SkinnedRenderMeshInstance::GetSkinnedPositionBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceChildHandle<wand::Buffer> {
  assert(frame_idx < kFramesInFlight);
  return skinned_pos_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetSkinnedNormalBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceChildHandle<wand::Buffer> {
  assert(frame_idx < kFramesInFlight);
  return skinned_norm_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetSkinnedTangentBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceChildHandle<wand::Buffer> {
  assert(frame_idx < kFramesInFlight);
  return skinned_tan_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetBoneMatrixBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceChildHandle<wand::Buffer> {
  assert(frame_idx < kFramesInFlight);
  return bone_mtx_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetVertexCapacity() const -> std::uint64_t {
  return vtx_capacity_;
}


auto SkinnedRenderMeshInstance::GetBoneCapacity() const -> std::uint64_t {
  return bone_capacity_;
}


auto SkinnedRenderMeshInstance::GetLastSkinningFrame() const -> std::optional<std::uint64_t> {
  return last_skinning_frame_;
}


auto SkinnedRenderMeshInstance::SetLastSkinningFrame(std::uint64_t const frame) -> void {
  last_skinning_frame_ = frame;
}


auto SkinnedRenderMeshInstance::InvalidateSkinningHistory() -> void {
  last_skinning_frame_.reset();
}
}
