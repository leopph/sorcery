#include "Mesh.hpp"

#include "../app.hpp"
#include "../Serialization.hpp"
#include "../rendering/render_manager.hpp"

#include <DirectXMesh.h>
#include <imgui.h>

#include <algorithm>
#include <cassert>
#include <utility>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Mesh>{"Mesh"};
}


namespace sorcery {
Submesh::Submesh(SubmeshData const& data) :
  first_meshlet_{data.first_meshlet},
  meshlet_count_{data.meshlet_count},
  material_idx_{data.material_idx} {}


auto Submesh::GetFirstMeshlet() const -> std::uint32_t {
  return first_meshlet_;
}


auto Submesh::GetMeshletCount() const -> std::uint32_t {
  return meshlet_count_;
}


auto Submesh::GetMaterialIndex() const -> std::uint32_t {
  return material_idx_;
}


auto Mesh::OnDrawProperties(bool& changed) -> void {
  Resource::OnDrawProperties(changed);

  ImGui::Text("%s: %d", "Vertex Count", vertex_count_);
  ImGui::Text("%s: %d", "Submesh Count", submeshes_.size());
  ImGui::Text("%s: %d", "Meshlet Count", meshlets_.size());
}


Mesh::Mesh(MeshData const& data) noexcept {
  SetData(data);
}


auto Mesh::SetData(MeshData const& data) noexcept -> void {
  // GPU buffers

  pos_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.positions.size() * sizeof(Vector4)), .stride = sizeof(Vector4),
    .constant_buffer = false, .shader_resource = true, .unordered_access = true
  }, D3D12_HEAP_TYPE_DEFAULT);

  norm_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.normals.size() * sizeof(Vector4)), .stride = sizeof(Vector4),
    .constant_buffer = false, .shader_resource = true, .unordered_access = true
  }, D3D12_HEAP_TYPE_DEFAULT);

  tan_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.tangents.size() * sizeof(Vector4)), .stride = sizeof(Vector4),
    .constant_buffer = false, .shader_resource = true, .unordered_access = true
  }, D3D12_HEAP_TYPE_DEFAULT);

  uv_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.uvs.size() * sizeof(Vector2)), .stride = sizeof(Vector2),
    .constant_buffer = false, .shader_resource = true, .unordered_access = false
  }, D3D12_HEAP_TYPE_DEFAULT);

  bone_weight_buf_ = data.bone_weights.empty()
                       ? nullptr
                       : App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
                         .size = static_cast<UINT>(data.bone_weights.size() * sizeof(Vector4)),
                         .stride = sizeof(Vector4),
                         .constant_buffer = false, .shader_resource = false, .unordered_access = true
                       }, D3D12_HEAP_TYPE_DEFAULT);

  bone_idx_buf_ = data.bone_indices.empty()
                    ? nullptr
                    : App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
                      .size = static_cast<UINT>(data.bone_indices.size() * sizeof(Vector<std::uint32_t, 4>)),
                      .stride = sizeof(Vector<std::uint32_t, 4>), .constant_buffer = false, .shader_resource = false,
                      .unordered_access = true
                    }, D3D12_HEAP_TYPE_DEFAULT);

  meshlet_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.meshlets.size() * sizeof(MeshletData)), .stride = sizeof(MeshletData),
    .constant_buffer = false, .shader_resource = true, .unordered_access = false
  }, D3D12_HEAP_TYPE_DEFAULT);

  vertex_idx_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.vertex_indices.size()), .stride = 1,
    .constant_buffer = false, .shader_resource = true, .unordered_access = false
  }, D3D12_HEAP_TYPE_DEFAULT);

  prim_idx_buf_ = App::Instance().GetGraphicsDevice().CreateBuffer(graphics::BufferDesc{
    .size = static_cast<UINT>(data.triangle_indices.size() * sizeof(MeshletTriangleData)),
    .stride = sizeof(MeshletTriangleData), .constant_buffer = false, .shader_resource = true,
    .unordered_access = false
  }, D3D12_HEAP_TYPE_DEFAULT);

  // CPU lists

  meshlets_ = data.meshlets;
  mtl_slots_ = data.material_slots;
  animations_ = data.animations;
  skeleton_ = data.skeleton;
  bones_ = data.bones;

  // Other info

  bounds_.min = Vector3{std::numeric_limits<float>::max()};
  bounds_.max = Vector3{std::numeric_limits<float>::lowest()};

  for (auto const& pos : data.positions) {
    bounds_.min = Min(bounds_.min, pos);
    bounds_.max = Max(bounds_.max, pos);
  }

  vertex_count_ = data.positions.size();
  primitive_count_ = data.triangle_indices.size();
}


auto Mesh::GetPositionBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return pos_buf_;
}


auto Mesh::GetNormalBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return norm_buf_;
}


auto Mesh::GetTangentBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return tan_buf_;
}


auto Mesh::GetUvBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return uv_buf_;
}


auto Mesh::GetBoneWeightBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_weight_buf_;
}


auto Mesh::GetBoneIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return bone_idx_buf_;
}


auto Mesh::GetMeshletBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return meshlet_buf_;
}


auto Mesh::GetVertexIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return vertex_idx_buf_;
}


auto Mesh::GetPrimitiveIndexBuffer() const -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return prim_idx_buf_;
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
