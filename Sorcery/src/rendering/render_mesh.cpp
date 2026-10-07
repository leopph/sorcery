#include "render_mesh.hpp"

#include "../mesh_data.hpp"
#include "../util.hpp"


namespace sorcery::rendering {
auto RenderMesh::Init(wand::GraphicsDevice& device, std::uint64_t const vtx_count, std::uint64_t const meshlet_count,
                      std::uint64_t const vtx_idx_byte_count, std::uint64_t const prim_idx_count,
                      std::uint64_t const cull_data_count, bool const has_skinning_data) -> void {
  using wand::BufferDesc;
  using wand::BufferViewDesc;
  using wand::BufferUsage;
  using wand::BufferViewUsage;

  pos_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = vtx_count * sizeof(Vector4),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination | BufferUsage::kCopySource
    }, BufferViewDesc{
      .offset = 0,
      .size = vtx_count * sizeof(Vector4),
      .stride = sizeof(Vector4),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  norm_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = vtx_count * sizeof(Vector4),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination | BufferUsage::kCopySource
    }, BufferViewDesc{
      .offset = 0,
      .size = vtx_count * sizeof(Vector4),
      .stride = sizeof(Vector4),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  tan_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = vtx_count * sizeof(Vector4),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination | BufferUsage::kCopySource
    }, BufferViewDesc{
      .offset = 0,
      .size = vtx_count * sizeof(Vector4),
      .stride = sizeof(Vector4),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  uv_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = vtx_count * sizeof(Vector2),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
    }, BufferViewDesc{
      .offset = 0,
      .size = vtx_count * sizeof(Vector2),
      .stride = sizeof(Vector2),
      .usage = BufferViewUsage::kShaderResource

    }, wand::CpuAccess::kNone);

  bone_weight_buf_ =
    has_skinning_data
      ? CreateBufferWithView(device,
        BufferDesc{
          .size = vtx_count * sizeof(Vector4),
          .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = vtx_count * sizeof(Vector4),
          .stride = sizeof(Vector4),
          .usage = BufferViewUsage::kShaderResource
        }, wand::CpuAccess::kNone)
      : nullptr;

  bone_idx_buf_ =
    has_skinning_data
      ? CreateBufferWithView(device,
        BufferDesc{
          .size = vtx_count * sizeof(Vector4U),
          .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
        }, BufferViewDesc{
          .offset = 0,
          .size = vtx_count * sizeof(Vector4U),
          .stride = sizeof(Vector4U),
          .usage = BufferViewUsage::kShaderResource
        }, wand::CpuAccess::kNone)
      : nullptr;

  meshlet_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = meshlet_count * sizeof(MeshletData),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
    }, BufferViewDesc{
      .offset = 0,
      .size = meshlet_count * sizeof(MeshletData),
      .stride = sizeof(MeshletData),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  vertex_idx_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = RoundToNextMultiple(vtx_idx_byte_count, 4ull),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
    }, BufferViewDesc{
      .offset = 0,
      .size = RoundToNextMultiple(vtx_idx_byte_count, 4ull),
      .stride = 1,
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  prim_idx_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = prim_idx_count * sizeof(MeshletTriangleData),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
    }, BufferViewDesc{
      .offset = 0,
      .size = prim_idx_count * sizeof(MeshletTriangleData),
      .stride = sizeof(MeshletTriangleData),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);

  cull_data_buf_ = CreateBufferWithView(device,
    BufferDesc{
      .size = cull_data_count * sizeof(MeshletCullData),
      .usage = BufferUsage::kShaderResource | BufferUsage::kCopyDestination
    }, BufferViewDesc{
      .offset = 0,
      .size = cull_data_count * sizeof(MeshletCullData),
      .stride = sizeof(MeshletCullData),
      .usage = BufferViewUsage::kShaderResource
    }, wand::CpuAccess::kNone);
}


auto RenderMesh::GetPositionBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return pos_buf_;
}


auto RenderMesh::GetNormalBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return norm_buf_;
}


auto RenderMesh::GetTangentBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return tan_buf_;
}


auto RenderMesh::GetUvBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return uv_buf_;
}


auto RenderMesh::GetBoneWeightBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return bone_weight_buf_;
}


auto RenderMesh::GetBoneIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return bone_idx_buf_;
}


auto RenderMesh::GetMeshletBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return meshlet_buf_;
}


auto RenderMesh::GetVertexIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return vertex_idx_buf_;
}


auto RenderMesh::GetPrimitiveIndexBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return prim_idx_buf_;
}


auto RenderMesh::GetCullDataBuffer() const -> wand::SharedDeviceHandle<wand::BufferView> const& {
  return cull_data_buf_;
}


auto RenderMesh::GetRevision() const -> std::uint64_t {
  return revision_;
}


auto RenderMesh::SetRevision(std::uint64_t const rev) -> void {
  revision_ = rev;
}
}
