#pragma once

#include <type_traits>

#include <DirectXMesh.h>


namespace sorcery {
template<typename IdxType, typename PosType>
  requires(std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>)
          && (std::same_as<PosType, Vector3> || std::same_as<PosType, Vector4>)
auto ComputeMeshlets(std::span<IdxType const> const indices, std::span<PosType const> const positions,
                     std::vector<MeshletData>& out_meshlets,
                     std::vector<std::uint8_t>& out_unique_vertex_indices,
                     std::vector<MeshletTriangleData>& out_primitive_indices,
                     std::vector<MeshletCullData>& out_cull_data,
                     std::uint16_t const max_verts_per_meshlet,
                     std::uint16_t const max_prims_per_meshlet) -> bool {
  using DxPosType = std::conditional_t<std::same_as<PosType, Vector3>, DirectX::XMFLOAT3, DirectX::XMFLOAT4>;

  std::vector<DxPosType> dx_positions;
  dx_positions.reserve(std::size(positions));
  std::ranges::transform(positions, std::back_inserter(dx_positions), [](auto const& v) {
    return *std::bit_cast<DxPosType const*>(&v);
  });

  std::vector<DirectX::Meshlet> dx_meshlets;
  std::vector<DirectX::MeshletTriangle> dx_primitives;

  auto ret{
    SUCCEEDED(
      DirectX::ComputeMeshlets(indices.data(), std::size(indices) / 3, dx_positions.data(), std::size(dx_positions),
        nullptr, dx_meshlets, out_unique_vertex_indices, dx_primitives,max_verts_per_meshlet, max_prims_per_meshlet))
  };

  if (ret) {
    std::vector<DirectX::CullData> dx_cull_data;
    dx_cull_data.resize(std::size(dx_meshlets));

    ret = SUCCEEDED(
      DirectX::ComputeCullData(dx_positions.data(), std::size(dx_positions), dx_meshlets.data(), dx_meshlets.size(),
        std::bit_cast<IdxType*>(out_unique_vertex_indices.data()), std::size(out_unique_vertex_indices),
        dx_primitives.data(), std::size(dx_primitives), dx_cull_data.data()));

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

      out_cull_data.clear();
      out_cull_data.reserve(std::size(dx_cull_data));

      std::ranges::transform(dx_cull_data, std::back_inserter(out_cull_data),
        [](DirectX::CullData const& cull_data) {
          return MeshletCullData{
            .bounding_sphere = BoundingSphere{
              Vector3{
                cull_data.BoundingSphere.Center.x, cull_data.BoundingSphere.Center.y, cull_data.BoundingSphere.Center.z
              },
              cull_data.BoundingSphere.Radius
            },
            .normal_cone = Vector<std::uint8_t, 4>{
              cull_data.NormalCone.x, cull_data.NormalCone.y,
              cull_data.NormalCone.z, cull_data.NormalCone.w
            },
            .apex_offset = cull_data.ApexOffset,
          };
        });
    }
  }

  return ret;
}
}
