#include "skinned_render_mesh_instance.hpp"

#include <cassert>


namespace sorcery::rendering {
auto SkinnedRenderMeshInstance::Init(wand::GraphicsDevice& device, std::uint64_t const vtx_count,
                                     std::uint64_t const bone_count) -> void {
  vtx_capacity_ = vtx_count;
  bone_capacity_ = bone_count;

  for (auto i = 0u; i < kFramesInFlight; i++) {
    using wand::BufferDesc;
    using wand::BufferViewDesc;
    using wand::BufferUsage;
    using wand::BufferViewUsage;

    if (vtx_count != 0) {
      skinned_pos_buf_[i] = CreateBufferWithView(device,
        BufferDesc{
          .size = vtx_count * sizeof(Vector4),
          .usage = BufferUsage::kShaderResource | BufferUsage::kUnorderedAccess |
                   BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = vtx_count * sizeof(Vector4),
          .stride = sizeof(Vector4),
          .usage = BufferViewUsage::kShaderResource | BufferViewUsage::kUnorderedAccess
        }, wand::CpuAccess::kNone);

      skinned_norm_buf_[i] = CreateBufferWithView(device,
        BufferDesc{
          .size = vtx_count * sizeof(Vector4),
          .usage = BufferUsage::kShaderResource | BufferUsage::kUnorderedAccess |
                   BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = vtx_count * sizeof(Vector4),
          .stride = sizeof(Vector4),
          .usage = BufferViewUsage::kShaderResource | BufferViewUsage::kUnorderedAccess
        }, wand::CpuAccess::kNone);

      skinned_tan_buf_[i] = CreateBufferWithView(device,
        BufferDesc{
          .size = vtx_count * sizeof(Vector4),
          .usage = BufferUsage::kShaderResource | BufferUsage::kUnorderedAccess |
                   BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = vtx_count * sizeof(Vector4),
          .stride = sizeof(Vector4),
          .usage = BufferViewUsage::kShaderResource | BufferViewUsage::kUnorderedAccess
        }, wand::CpuAccess::kNone);
    } else {
      skinned_pos_buf_[i].reset();
      skinned_norm_buf_[i].reset();
      skinned_tan_buf_[i].reset();
    }

    if (bone_count != 0) {
      bone_mtx_buf_[i] = CreateBufferWithView(device,
        BufferDesc{
          .size = bone_count * sizeof(Matrix4),
          .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = bone_count * sizeof(Matrix4),
          .stride = sizeof(Matrix4),
          .usage = BufferViewUsage::kShaderResource
        }, wand::CpuAccess::kNone);
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
  unsigned const frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView> {
  assert(frame_idx < kFramesInFlight);
  return skinned_pos_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetSkinnedNormalBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView> {
  assert(frame_idx < kFramesInFlight);
  return skinned_norm_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetSkinnedTangentBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView> {
  assert(frame_idx < kFramesInFlight);
  return skinned_tan_buf_[frame_idx];
}


auto SkinnedRenderMeshInstance::GetBoneMatrixBuffer(
  unsigned const frame_idx) const -> wand::SharedDeviceHandle<wand::BufferView> {
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
