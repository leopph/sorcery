#include "Mesh.hpp"

#include "../app.hpp"
#include "../Serialization.hpp"
#include "../rendering/render_manager.hpp"

#include <DirectXMesh.h>
#include <imgui.h>

#include <algorithm>
#include <bit>
#include <cassert>
#include <iterator>
#include <utility>


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


auto Mesh::OnDrawProperties(bool& changed) -> void {
  Resource::OnDrawProperties(changed);

  ImGui::Text("%s: %d", "Vertex Count", vertex_count_);
  ImGui::Text("%s: %d", "Triangle Count", primitive_count_);
  ImGui::Text("%s: %d", "Meshlet Count", meshlets_.size());
  ImGui::Text("%s: %d", "Submesh Count", submeshes_.size());
  ImGui::Text("%s: %s", "Uses 32-bit indices", idx32_ ? "yes" : "no");
}


Mesh::Mesh(MeshData const& data) noexcept {
  SetData(data);
}


auto Mesh::SetData(MeshData const& data) noexcept -> void {
  // GPU buffers

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

  pos_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.positions, 1, vec4_buf), false, true, true);
  norm_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.normals, 0, vec4_buf), false, true, true);
  tan_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.tangents, 0, vec4_buf), false, true, true);
  uv_buf_ = StructuredBuffer<Vector2>::New(gd, rm, data.uvs, false, true, false);
  bone_weight_buf_ = data.bone_weights.empty()
                       ? StructuredBuffer<Vector4>{}
                       : StructuredBuffer<Vector4>::New(gd, rm, data.bone_weights, false, false, true);
  bone_idx_buf_ = data.bone_indices.empty()
                    ? StructuredBuffer<Vector<std::uint32_t, 4>>{}
                    : StructuredBuffer<Vector<std::uint32_t, 4>>::New(gd, rm, data.bone_indices, false, false, true);
  meshlet_buf_ = StructuredBuffer<MeshletData>::New(gd, rm, data.meshlets, false);
  vertex_idx_buf_ = gd.CreateBuffer(graphics::BufferDesc{data.vertex_indices.size(), 1, false, true, false},
    graphics::CpuAccess::kNone);
  prim_idx_buf_ = StructuredBuffer<MeshletTriangleData>::New(gd, rm, data.triangle_indices, false, true, false);
  cull_data_buf_ = StructuredBuffer<MeshletCullData>::New(gd, rm, data.cull_data, false, true, false);

  rm.UpdateBuffer(*vertex_idx_buf_, 0, as_bytes(std::span{data.vertex_indices}));

  // CPU lists

  meshlets_ = data.meshlets;
  mtl_slots_ = data.material_slots;

  submeshes_.clear();
  submeshes_.reserve(data.submeshes.size());
  std::ranges::for_each(data.submeshes, [this](SubmeshData const& submesh) {
    submeshes_.emplace_back(submesh);
  });

  animations_ = data.animations;
  skeleton_ = data.skeleton;
  bones_ = data.bones;

  // Other info

  bounds_ = data.bounds;
  vertex_count_ = data.positions.size();
  primitive_count_ = data.triangle_indices.size();
  idx32_ = data.idx32;
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


auto Mesh::Has32BitVertexIndices() const noexcept -> bool {
  return idx32_;
}
}
