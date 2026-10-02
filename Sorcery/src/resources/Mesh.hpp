#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "Resource.hpp"
#include "../Bounds.hpp"
#include "../Math.hpp"
#include "../mesh_data.hpp"
#include "../observer_ptr.hpp"
#include "../resource_residency_policy.hpp"


namespace sorcery {
class Mesh;


namespace detail {
auto ClearMeshCpuData(Mesh& mesh) -> void;
}


class Submesh {
public:
  explicit Submesh(SubmeshData const& data);

  [[nodiscard]] SORCERYAPI
  auto GetFirstMeshlet() const -> std::uint32_t;
  [[nodiscard]] SORCERYAPI
  auto GetMeshletCount() const -> std::uint32_t;
  [[nodiscard]] SORCERYAPI
  auto GetBaseVertex() const -> std::uint32_t;
  [[nodiscard]] SORCERYAPI
  auto GetMaterialIndex() const -> std::uint32_t;
  [[nodiscard]] SORCERYAPI
  auto GetBounds() const -> AABB const&;

private:
  std::uint32_t first_meshlet_;
  std::uint32_t meshlet_count_;
  std::uint32_t base_vertex_;
  std::uint32_t material_idx_;
  AABB bounds_;
};


class Mesh final : public Resource {
  RTTR_ENABLE(Resource)

public:
  Mesh() = default;
  Mesh(Mesh const&) = delete;
  Mesh(Mesh&& other) noexcept = delete;
  SORCERYAPI Mesh(MeshData data, CpuResidencyPolicy cpu_data_policy);

  ~Mesh() override = default;

  auto operator=(Mesh const&) -> void = delete;
  auto operator=(Mesh&& other) noexcept -> void = delete;

  [[nodiscard]] SORCERYAPI
  auto GetData() const -> ObserverPtr<MeshData const>;
  SORCERYAPI
  auto SetData(MeshData data, CpuResidencyPolicy cpu_data_policy) -> void;

  [[nodiscard]] SORCERYAPI
  auto GetCpuDataPolicy() const -> CpuResidencyPolicy;

  [[nodiscard]] SORCERYAPI
  auto GetMaterialSlots() const noexcept -> std::span<MaterialSlotInfo const>;
  [[nodiscard]] SORCERYAPI
  auto GetSubmeshes() const noexcept -> std::span<Submesh const>;
  [[nodiscard]] SORCERYAPI
  auto GetAnimations() const noexcept -> std::span<Animation const>;
  [[nodiscard]] SORCERYAPI
  auto GetSkeleton() const noexcept -> std::span<SkeletonNode const>;
  [[nodiscard]] SORCERYAPI
  auto GetBones() const noexcept -> std::span<Bone const>;

  [[nodiscard]] SORCERYAPI
  auto GetBounds() const noexcept -> AABB const&;
  [[nodiscard]] SORCERYAPI
  auto GetVertexCount() const noexcept -> std::size_t;
  [[nodiscard]] SORCERYAPI
  auto GetPrimitiveCount() const noexcept -> std::size_t;
  [[nodiscard]] SORCERYAPI
  auto GetMeshletCount() const noexcept -> std::size_t;
  [[nodiscard]] SORCERYAPI
  auto Has32BitVertexIndices() const noexcept -> bool;

  [[nodiscard]] SORCERYAPI
  auto GetRevision() const noexcept -> std::uint64_t;

private:
  std::unique_ptr<MeshData> mesh_data_;

  std::vector<MaterialSlotInfo> mtl_slots_;
  std::vector<Submesh> submeshes_;
  std::vector<Animation> animations_;
  std::vector<SkeletonNode> skeleton_;
  std::vector<Bone> bones_;
  AABB bounds_;
  std::size_t vertex_count_{0};
  std::size_t primitive_count_{0};
  std::size_t meshlet_count_{0};
  std::uint64_t revision_{};
  bool idx32_{false};
  CpuResidencyPolicy cpu_data_policy_{CpuResidencyPolicy::kReleaseAfterUpload};

  friend auto detail::ClearMeshCpuData(Mesh& mesh) -> void;
};


struct SubmeshFaceRange {
  std::size_t first_face;
  std::size_t face_count;
};


struct SubmeshMeshletRange {
  std::size_t first_meshlet;
  std::size_t meshlet_count;
};


template<typename IdxType, typename PosType>
  requires (std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>)
           && (std::same_as<PosType, Vector3> || std::same_as<PosType, Vector4>)
[[nodiscard]] auto ComputeMeshlets(std::span<IdxType const> indices, std::span<PosType const> positions,
                                   std::vector<MeshletData>& out_meshlets,
                                   std::vector<std::uint8_t>& out_unique_vertex_indices,
                                   std::vector<MeshletTriangleData>& out_primitive_indices,
                                   std::vector<MeshletCullData>& out_cull_data,
                                   std::uint16_t max_verts_per_meshlet = kMeshletMaxVerts,
                                   std::uint16_t max_prims_per_meshlet = kMeshletMaxPrims) -> bool;
}


#include "mesh.inl"
