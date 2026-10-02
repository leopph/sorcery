#include "render_mesh.hpp"

#include "../mesh_data.hpp"
#include "wand/flags.hpp"


namespace sorcery::rendering {
auto RenderMesh::Init(wand::GraphicsDevice& device, std::uint64_t const vtx_count, std::uint64_t const meshlet_count,
                      std::uint64_t const vtx_idx_byte_count, std::uint64_t const prim_idx_count,
                      std::uint64_t const cull_data_count, bool const has_skinning_data) -> void {
  // We only need UAV access if the mesh can be skinned.
  auto const geom_buf_usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination |
                              (has_skinning_data ? wand::BufferUsage::kUnorderedAccess : wand::BufferUsage::kNone);

  pos_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = vtx_count * sizeof(Vector4),
    .stride = sizeof(Vector4),
    .usage = geom_buf_usage
  }, wand::CpuAccess::kNone);

  norm_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = vtx_count * sizeof(Vector4),
    .stride = sizeof(Vector4),
    .usage = geom_buf_usage
  }, wand::CpuAccess::kNone);

  tan_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = vtx_count * sizeof(Vector4),
    .stride = sizeof(Vector4),
    .usage = geom_buf_usage
  }, wand::CpuAccess::kNone);

  uv_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = vtx_count * sizeof(Vector2),
    .stride = sizeof(Vector2),
    .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);

  bone_weight_buf_ =
    has_skinning_data
      ? device.CreateBuffer(wand::BufferDesc{
        .size = vtx_count * sizeof(Vector4),
        .stride = sizeof(Vector4),
        .usage = wand::BufferUsage::kUnorderedAccess | wand::BufferUsage::kCopyDestination
      }, wand::CpuAccess::kNone)
      : nullptr;

  bone_idx_buf_ =
    has_skinning_data
      ? device.CreateBuffer(wand::BufferDesc{
        .size = vtx_count * sizeof(Vector4U),
        .stride = sizeof(Vector4U),
        .usage = wand::BufferUsage::kUnorderedAccess | wand::BufferUsage::kCopyDestination
      }, wand::CpuAccess::kNone)
      : nullptr;

  meshlet_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = meshlet_count * sizeof(MeshletData),
    .stride = sizeof(MeshletData),
    .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);

  vertex_idx_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = vtx_idx_byte_count,
    .stride = 1,
    .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);

  prim_idx_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = prim_idx_count * sizeof(MeshletTriangleData),
    .stride = sizeof(MeshletTriangleData),
    .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);

  cull_data_buf_ = device.CreateBuffer(wand::BufferDesc{
    .size = cull_data_count * sizeof(MeshletCullData),
    .stride = sizeof(MeshletCullData),
    .usage = wand::BufferUsage::kShaderResource | wand::BufferUsage::kCopyDestination
  }, wand::CpuAccess::kNone);
}


auto RenderMesh::GetPositionBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return pos_buf_;
}


auto RenderMesh::GetNormalBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return norm_buf_;
}


auto RenderMesh::GetTangentBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return tan_buf_;
}


auto RenderMesh::GetUvBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return uv_buf_;
}


auto RenderMesh::GetBoneWeightBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return bone_weight_buf_;
}


auto RenderMesh::GetBoneIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return bone_idx_buf_;
}


auto RenderMesh::GetMeshletBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return meshlet_buf_;
}


auto RenderMesh::GetVertexIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return vertex_idx_buf_;
}


auto RenderMesh::GetPrimitiveIndexBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return prim_idx_buf_;
}


auto RenderMesh::GetCullDataBuffer() const -> wand::SharedDeviceChildHandle<wand::Buffer> const& {
  return cull_data_buf_;
}


auto RenderMesh::GetRevision() const -> std::uint64_t {
  return revision_;
}


auto RenderMesh::SetRevision(std::uint64_t const rev) -> void {
  revision_ = rev;
}
}
