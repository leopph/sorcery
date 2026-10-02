#include "Mesh.hpp"

#include <algorithm>
#include <stdexcept>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{"Mesh"};
}


namespace sorcery {
auto detail::ClearMeshCpuData(Mesh& mesh) -> void {
  mesh.mesh_data_.reset();
}


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


Mesh::Mesh(MeshData data, CpuResidencyPolicy const cpu_data_policy) {
  SetData(std::move(data), cpu_data_policy);
}


auto Mesh::GetData() const -> ObserverPtr<MeshData const> {
  return MakeObserver(mesh_data_.get());
}


auto Mesh::SetData(MeshData data, CpuResidencyPolicy const cpu_data_policy) -> void {
  if (data.positions.size() != data.normals.size() ||
      data.positions.size() != data.tangents.size() ||
      data.positions.size() != data.uvs.size()) {
    throw std::runtime_error{"Inconsistent vertex data sizes."};
  }

  if ((!data.bone_weights.empty() || !data.bone_indices.empty()) &&
      (data.positions.size() != data.bone_weights.size() ||
       data.positions.size() != data.bone_indices.size())) {
    throw std::runtime_error{"Inconsistent skinning data sizes."};
  }

  mesh_data_ = std::make_unique<MeshData>(std::move(data));

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
  meshlet_count_ = mesh_data_->meshlets.size();
  idx32_ = mesh_data_->idx32;

  cpu_data_policy_ = cpu_data_policy;

  ++revision_;
}


auto Mesh::GetCpuDataPolicy() const -> CpuResidencyPolicy {
  return cpu_data_policy_;
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
  return meshlet_count_;
}


auto Mesh::Has32BitVertexIndices() const noexcept -> bool {
  return idx32_;
}


auto Mesh::GetRevision() const noexcept -> std::uint64_t {
  return revision_;
}
}
