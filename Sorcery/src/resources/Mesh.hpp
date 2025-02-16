#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <variant>
#include <vector>

#include "Resource.hpp"
#include "../Bounds.hpp"
#include "../Math.hpp"
#include "../mesh_data.hpp"
#include "../rendering/graphics.hpp"


namespace sorcery {
class Submesh {
public:
  explicit Submesh(SubmeshData const& data);
  Submesh(Submesh const&) = default;
  Submesh(Submesh&&) noexcept = default;

  ~Submesh();

  auto operator=(Submesh const&) -> Submesh& = default;
  auto operator=(Submesh&&) noexcept -> Submesh& = default;

  [[nodiscard]] LEOPPHAPI auto GetPositionBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto GetNormalBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto GetTangentBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto GetUvBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto
  GetBoneWeightBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto GetBoneIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto GetMeshletBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto
  GetVertexIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;
  [[nodiscard]] LEOPPHAPI auto
  GetPrimitiveIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const&;

  [[nodiscard]] auto GetBounds() const -> AABB const&;

  [[nodiscard]] auto GetVertexCount() const -> std::size_t;
  [[nodiscard]] auto GetPrimitiveCount() const -> std::size_t;
  [[nodiscard]] auto GetMeshletCount() const -> std::size_t;

private:
  // Geometry

  graphics::SharedDeviceChildHandle<graphics::Buffer> pos_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> norm_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> tan_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> uv_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> bone_weight_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> bone_idx_buf_;

  // Indexing

  graphics::SharedDeviceChildHandle<graphics::Buffer> meshlet_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> vertex_idx_buf_;
  graphics::SharedDeviceChildHandle<graphics::Buffer> prim_idx_buf_;

  // CPU info

  std::vector<MeshletData> meshlets_;
  AABB bounds_;
  std::size_t vertex_count_;
  std::size_t primitive_count_;
  std::uint32_t material_idx_;
};


class Mesh final : public Resource {
  RTTR_ENABLE(Resource)
  std::vector<MaterialSlotInfo> mtl_slots_;
  std::vector<Submesh> submeshes_;
  std::vector<Animation> animations_;
  std::vector<SkeletonNode> skeleton_;
  std::vector<Bone> bones_;

  AABB bounds_;
  std::size_t vertex_count_{0};
  std::size_t primitive_count_{0};
  std::size_t meshlet_count_{0};

public:
  LEOPPHAPI auto OnDrawProperties(bool& changed) -> void override;

  Mesh() = default;
  Mesh(Mesh const&) = delete;
  Mesh(Mesh&& other) noexcept = delete;
  LEOPPHAPI explicit Mesh(MeshData const& data) noexcept;

  ~Mesh() override = default;

  auto operator=(Mesh const&) -> void = delete;
  auto operator=(Mesh&& other) noexcept -> void = delete;

  LEOPPHAPI auto SetData(MeshData const& data) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetMaterialSlots() const noexcept -> std::span<MaterialSlotInfo const>;
  [[nodiscard]] LEOPPHAPI auto GetSubmeshes() const noexcept -> std::span<Submesh const>;
  [[nodiscard]] LEOPPHAPI auto GetAnimations() const noexcept -> std::span<Animation const>;
  [[nodiscard]] LEOPPHAPI auto GetSkeleton() const noexcept -> std::span<SkeletonNode const>;
  [[nodiscard]] LEOPPHAPI auto GetBones() const noexcept -> std::span<Bone const>;

  [[nodiscard]] LEOPPHAPI auto GetBounds() const noexcept -> AABB const&;
  [[nodiscard]] LEOPPHAPI auto GetVertexCount() const noexcept -> std::size_t;
  [[nodiscard]] LEOPPHAPI auto GetPrimitiveCount() const noexcept -> std::size_t;
  [[nodiscard]] LEOPPHAPI auto GetMeshletCount() const noexcept -> std::size_t;
};


[[nodiscard]] LEOPPHAPI auto ComputeMeshlets(
  std::variant<std::span<std::uint16_t const>, std::span<std::uint32_t const>> const& indices,
  std::variant<std::span<Vector3 const>, std::span<Vector4 const>> const& positions,
  std::vector<MeshletData>& out_meshlets,
  std::vector<std::uint8_t>& out_unique_vertex_indices,
  std::vector<MeshletTriangleData>& out_primitive_indices,
  std::uint16_t max_verts_per_meshlet = kMeshletMaxVerts,
  std::uint16_t max_prims_per_meshlet = kMeshletMaxPrims) -> bool;
}
