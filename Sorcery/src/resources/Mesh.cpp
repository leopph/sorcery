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

  auto const to_index_format{
    []<typename Format>(std::span<std::uint8_t const> const indices) {
      return std::span{std::bit_cast<Format const*>(indices.data()), indices.size() / sizeof(Format)};
    }
  };

  auto const to_u32{
    [to_index_format](std::span<std::uint8_t const> const indices) {
      return to_index_format.operator()<std::uint32_t>(indices);
    }
  };

  auto const to_u16{
    [to_index_format](std::span<std::uint8_t const> const indices) {
      return to_index_format.operator()<std::uint16_t>(indices);
    }
  };

  std::vector<Vector4> vec4_buf;

  auto& gd{App::Instance().GetGraphicsDevice()};
  auto& rm{App::Instance().GetRenderManager()};

  using rendering::StructuredBuffer;

  pos_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.positions, 1, vec4_buf), true, true);
  norm_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.normals, 0, vec4_buf), true, true);
  tan_buf_ = StructuredBuffer<Vector4>::New(gd, rm, to_vec4(data.tangents, 0, vec4_buf), true, true);
  uv_buf_ = StructuredBuffer<Vector2>::New(gd, rm, data.uvs, true, false);
  bone_weight_buf_ = data.bone_weights.empty()
                       ? StructuredBuffer<Vector4>{}
                       : StructuredBuffer<Vector4>::New(gd, rm, data.bone_weights, false, true);
  bone_idx_buf_ = data.bone_indices.empty()
                    ? StructuredBuffer<Vector<std::uint32_t, 4>>{}
                    : StructuredBuffer<Vector<std::uint32_t, 4>>::New(gd, rm, data.bone_indices, false, true);
  meshlet_buf_ = StructuredBuffer<MeshletData>::New(gd, rm, data.meshlets);
  if (data.idx32) {
    vertex_idx_buf_ = StructuredBuffer<std::uint32_t>::New(gd, rm, to_u32(data.vertex_indices), true, false);
  } else {
    vertex_idx_buf_ = StructuredBuffer<std::uint16_t>::New(gd, rm, to_u16(data.vertex_indices), true, false);
  }
  prim_idx_buf_ = StructuredBuffer<MeshletTriangleData>::New(gd, rm, data.triangle_indices, true, false);

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
  return std::visit<graphics::SharedDeviceChildHandle<graphics::Buffer> const&>([](auto const& buf) -> auto& {
    return buf.GetBuffer();
  }, vertex_idx_buf_);
}


auto Mesh::GetPrimitiveIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return prim_idx_buf_.GetBuffer();
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


auto ComputeMeshlets(std::variant<std::span<std::uint16_t const>, std::span<std::uint32_t const>> const& indices,
                     std::variant<std::span<Vector3 const>, std::span<Vector4 const>> const& positions,
                     std::span<SubmeshFaceRange const> submeshes, std::vector<MeshletData>& out_meshlets,
                     std::vector<std::uint8_t>& out_unique_vertex_indices,
                     std::vector<MeshletTriangleData>& out_primitive_indices,
                     std::vector<SubmeshMeshletRange>& out_submeshes, std::uint16_t max_verts_per_meshlet,
                     std::uint16_t max_prims_per_meshlet) -> bool {
  auto ret{false};

  std::visit(
    [&positions, submeshes, &ret, &out_meshlets, &out_unique_vertex_indices, &out_primitive_indices,
      &out_submeshes, max_verts_per_meshlet, max_prims_per_meshlet](auto const& index_span) {
      std::vector<DirectX::XMFLOAT3> dx_positions;

      std::visit([&dx_positions](auto const& position_span) {
        dx_positions.reserve(std::size(position_span));
        std::ranges::transform(position_span, std::back_inserter(dx_positions), [](auto const& v) {
          return DirectX::XMFLOAT3{v[0], v[1], v[2]};
        });
      }, positions);

      std::vector<std::pair<std::size_t, std::size_t>> dx_subsets;
      dx_subsets.reserve(submeshes.size());
      std::ranges::transform(submeshes, std::back_inserter(dx_subsets), [](SubmeshFaceRange const& range) {
        return std::pair{range.first_face, range.face_count};
      });

      std::vector<DirectX::Meshlet> dx_meshlets;
      std::vector<DirectX::MeshletTriangle> dx_primitives;
      std::vector<std::pair<std::size_t, std::size_t>> dx_meshlet_subsets(submeshes.size());

      ret = SUCCEEDED(DirectX::ComputeMeshlets(
        index_span.data(), std::size(index_span) / 3, dx_positions.data(), std::size(dx_positions), dx_subsets.data(),
        dx_subsets.size(), nullptr, dx_meshlets, out_unique_vertex_indices, dx_primitives,
        dx_meshlet_subsets.data(), max_verts_per_meshlet, max_prims_per_meshlet));

      if (ret) {
        out_meshlets.clear();
        out_meshlets.reserve(std::size(dx_meshlets));

        std::ranges::transform(dx_meshlets, std::back_inserter(out_meshlets), [](DirectX::Meshlet const& meshlet) {
          return MeshletData{
            .vert_count = meshlet.VertCount, .vert_offset = meshlet.VertOffset, .prim_count = meshlet.PrimCount,
            .prim_offset = meshlet.PrimOffset,
          };
        });

        out_primitive_indices.clear();
        out_primitive_indices.reserve(std::size(dx_primitives));

        std::ranges::transform(dx_primitives, std::back_inserter(out_primitive_indices),
          [](DirectX::MeshletTriangle const& tri) {
            return MeshletTriangleData{
              .idx0 = tri.i0, .idx1 = tri.i1, .idx2 = tri.i2,
            };
          });

        out_submeshes.clear();
        out_submeshes.reserve(std::size(dx_meshlet_subsets));

        std::ranges::transform(dx_meshlet_subsets, std::back_inserter(out_submeshes),
          [](std::pair<std::size_t, std::size_t> const& subset) {
            return SubmeshMeshletRange{
              .first_meshlet = subset.first, .meshlet_count = subset.second,
            };
          });
      }
    }, indices);

  return ret;
}


auto ComputeMeshlets(std::variant<std::span<std::uint16_t const>, std::span<std::uint32_t const>> const& indices,
                     std::variant<std::span<Vector3 const>, std::span<Vector4 const>> const& positions,
                     std::vector<MeshletData>& out_meshlets,
                     std::vector<std::uint8_t>& out_unique_vertex_indices,
                     std::vector<MeshletTriangleData>& out_primitive_indices,
                     std::uint16_t const max_verts_per_meshlet, std::uint16_t const max_prims_per_meshlet) -> bool {
  auto ret{false};

  std::visit(
    [&positions, &ret, &out_meshlets, &out_unique_vertex_indices, &out_primitive_indices, max_verts_per_meshlet,
      max_prims_per_meshlet](auto const& index_span) {
      std::vector<DirectX::XMFLOAT3> dx_positions;

      std::visit([&dx_positions](auto const& position_span) {
        dx_positions.reserve(std::size(position_span));
        std::ranges::transform(position_span, std::back_inserter(dx_positions), [](auto const& v) {
          return DirectX::XMFLOAT3{v[0], v[1], v[2]};
        });
      }, positions);

      std::vector<DirectX::Meshlet> dx_meshlets;
      std::vector<DirectX::MeshletTriangle> dx_primitives;

      ret = SUCCEEDED(DirectX::ComputeMeshlets(
        index_span.data(), std::size(index_span) / 3, dx_positions.data(), std::size(dx_positions), nullptr,
        dx_meshlets, out_unique_vertex_indices, dx_primitives, max_verts_per_meshlet, max_prims_per_meshlet));

      if (ret) {
        out_meshlets.clear();
        out_meshlets.reserve(std::size(dx_meshlets));

        std::ranges::transform(dx_meshlets, std::back_inserter(out_meshlets), [](DirectX::Meshlet const& meshlet) {
          return MeshletData{
            .vert_count = meshlet.VertCount, .vert_offset = meshlet.VertOffset, .prim_count = meshlet.PrimCount,
            .prim_offset = meshlet.PrimOffset,
          };
        });

        out_primitive_indices.clear();
        out_primitive_indices.reserve(std::size(dx_primitives));

        std::ranges::transform(dx_primitives, std::back_inserter(out_primitive_indices),
          [](DirectX::MeshletTriangle const& tri) {
            return MeshletTriangleData{
              .idx0 = tri.i0, .idx1 = tri.i1, .idx2 = tri.i2,
            };
          });
      }
    }, indices);

  return ret;
}
}
