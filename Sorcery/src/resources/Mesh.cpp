#include "Mesh.hpp"

#include <algorithm>
#include <iterator>

#include <DirectXMesh.h>

#include "../app.hpp"
#include "../rendering/render_manager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{"Mesh"};
}


namespace sorcery {
Submesh::Submesh(SubmeshData const& data) :
  first_meshlet_{data.first_meshlet},
  meshlet_count_{data.meshlet_count},
  base_vertex_{data.base_vertex},
  material_idx_{data.material_idx},
  bounds_{data.bounds} {}


auto Submesh::GetFirstMeshlet() const -> std::uint32_t {
  return first_meshlet_;
}


auto Submesh::GetMeshletCount() const -> std::uint32_t {
  return meshlet_count_;
}


auto Submesh::GetBaseVertex() const -> std::uint32_t {
  return base_vertex_;
}


auto Submesh::GetMaterialIndex() const -> std::uint32_t {
  return material_idx_;
}


auto Submesh::GetBounds() const -> AABB const& {
  return bounds_;
}


Mesh::Mesh(MeshData data, ResourceResidencyPolicy const data_policy) {
  SetData(std::move(data), data_policy);
}


auto Mesh::SetData(MeshData data, ResourceResidencyPolicy const data_policy) -> void {
  // CPU data
  mesh_data_ = std::make_unique<MeshData>(std::move(data));

  meshlets_ = mesh_data_->meshlets;
  mtl_slots_ = mesh_data_->material_slots;

  submeshes_.clear();
  submeshes_.reserve(mesh_data_->submeshes.size());
  std::ranges::for_each(mesh_data_->submeshes, [this](SubmeshData const& submesh) {
    submeshes_.emplace_back(submesh);
  });

  animations_ = mesh_data_->animations;
  skeleton_ = mesh_data_->skeleton;
  bones_ = mesh_data_->bones;

  bounds_ = mesh_data_->bounds;
  vertex_count_ = mesh_data_->positions.size();
  primitive_count_ = mesh_data_->triangle_indices.size();
  idx32_ = mesh_data_->idx32;

  // GPU data
  if (data_policy.gpu == GpuResidencyPolicy::kMakeResident) {
    UploadToGpu(data_policy.cpu);
  }
}


auto Mesh::UploadToGpu(CpuResidencyPolicy const cpu_policy) -> void {
  if (!mesh_data_) {
    return;
  }

  auto const to_vec4{
    [](std::span<Vector3 const> const vectors, float const component4,
       std::vector<Vector4>& out) -> std::vector<Vector4>& {
      out.reserve(vectors.size());
      out.clear();

      std::ranges::transform(vectors, std::back_inserter(out), [component4](Vector3 const vec3) {
        return Vector4{vec3, component4};
      });

      return out;
    }
  };

  std::vector<Vector4> vec4_buf;

  auto& gd{App::Instance().GetGraphicsDevice()};
  auto& rm{App::Instance().GetRenderManager()};

  using rendering::StructuredBuffer;

  pos_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(mesh_data_->positions, 1, vec4_buf), false, true, true);
  norm_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(mesh_data_->normals, 0, vec4_buf), false, true, true);
  tan_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(mesh_data_->tangents, 0, vec4_buf), false, true, true);
  uv_buf_ = StructuredBuffer<Vector2>::New(gd, rm, mesh_data_->uvs, false, true, false);
  bone_weight_buf_ = mesh_data_->bone_weights.empty()
                       ? StructuredBuffer<Vector4>{}
                       : StructuredBuffer<Vector4>::New(gd, rm, mesh_data_->bone_weights, false, false, true);
  bone_idx_buf_ = mesh_data_->bone_indices.empty()
                    ? StructuredBuffer<Vector<std::uint32_t, 4>>{}
                    : StructuredBuffer<Vector<std::uint32_t, 4>>::New(gd, rm, mesh_data_->bone_indices, false, false,
                      true);
  meshlet_buf_ = StructuredBuffer<MeshletData>::New(gd, rm, mesh_data_->meshlets, false);
  vertex_idx_buf_ = gd.CreateBuffer(graphics::BufferDesc{mesh_data_->vertex_indices.size(), 1, false, true, false},
    graphics::CpuAccess::kNone);
  prim_idx_buf_ = StructuredBuffer<MeshletTriangleData>::New(gd, rm, mesh_data_->triangle_indices, false, true, false);
  cull_data_buf_ = StructuredBuffer<MeshletCullData>::New(gd, rm, mesh_data_->cull_data, false, true, false);

  rm.UpdateBuffer(*vertex_idx_buf_, 0, as_bytes(std::span{mesh_data_->vertex_indices}));

  if (cpu_policy == CpuResidencyPolicy::kReleaseAfterUpload) {
    mesh_data_.reset();
  }
}


auto Mesh::GetPositionBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return pos_buf_.GetBuffer();
}


auto Mesh::GetNormalBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return norm_buf_.GetBuffer();
}


auto Mesh::GetTangentBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return tan_buf_.GetBuffer();
}


auto Mesh::GetUvBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return uv_buf_.GetBuffer();
}


auto Mesh::GetBoneWeightBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_weight_buf_.GetBuffer();
}


auto Mesh::GetBoneIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_idx_buf_.GetBuffer();
}


auto Mesh::GetMeshletBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return meshlet_buf_.GetBuffer();
}


auto Mesh::GetVertexIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return vertex_idx_buf_;
}


auto Mesh::GetPrimitiveIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return prim_idx_buf_.GetBuffer();
}


auto Mesh::GetCullDataBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return cull_data_buf_.GetBuffer();
}


auto Mesh::GetMaterialSlots() const noexcept -> std::span<MaterialSlotInfo const> {
  return mtl_slots_;
}


auto Mesh::GetSubmeshes() const noexcept -> std::span<Submesh const> {
  return submeshes_;
}


auto Mesh::GetAnimations() const noexcept -> std::span<Animation const> {
  return animations_;
}


auto Mesh::GetSkeleton() const noexcept -> std::span<SkeletonNode const> {
  return skeleton_;
}


auto Mesh::GetBones() const noexcept -> std::span<Bone const> {
  return bones_;
}


auto Mesh::GetBounds() const noexcept -> AABB const& {
  return bounds_;
}


auto Mesh::GetVertexCount() const noexcept -> std::size_t {
  return vertex_count_;
}


auto Mesh::GetPrimitiveCount() const noexcept -> std::size_t {
  return primitive_count_;
}


auto Mesh::GetMeshletCount() const noexcept -> std::size_t {
  return meshlets_.size();
}


auto Mesh::Has32BitVertexIndices() const noexcept -> bool {
  return idx32_;
}
}
