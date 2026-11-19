#pragma once

#include <bit>
#include <iterator>
#include <type_traits>
#include <utility>

#include <DirectXMesh.h>


namespace sorcery {
template<typename IdxType, typename PosType>
  requires (std::same_as<IdxType, std::uint16_t> || std::same_as<IdxType, std::uint32_t>)
           && (std::same_as<PosType, Vector3> || std::same_as<PosType, Vector4>)
auto ComputeMeshlets(
  std::span<IdxType const> const indices,
  std::span<PosType const> const positions,
  std::uint16_t const max_verts_per_meshlet,
  std::uint16_t const max_prims_per_meshlet
) -> std::expected<ComputeMeshletsResult, ComputeMeshletsError> {
  using DxPosType = std::conditional_t<std::same_as<PosType, Vector3>, DirectX::XMFLOAT3, DirectX::XMFLOAT4>;

  auto dx_positions = std::vector<DxPosType>{};
  dx_positions.reserve(std::size(positions));

  std::ranges::transform(positions, std::back_inserter(dx_positions), [](auto const& v) {
    return std::bit_cast<DxPosType>(v);
  });

  auto dx_meshlets = std::vector<DirectX::Meshlet>{};
  auto dx_primitives = std::vector<DirectX::MeshletTriangle>{};
  auto unique_vertex_indices = std::vector<std::uint8_t>{};

  auto hr = DirectX::ComputeMeshlets(indices.data(), indices.size() / 3, dx_positions.data(), dx_positions.size(),
    nullptr, dx_meshlets, unique_vertex_indices, dx_primitives, max_verts_per_meshlet, max_prims_per_meshlet);

  auto const to_error = [](auto const hresult) {
    if (hresult == E_INVALIDARG) {
      return std::unexpected{ComputeMeshletsError::kInvalidArg};
    }

    if (hresult == E_OUTOFMEMORY) {
      return std::unexpected{ComputeMeshletsError::kOutOfMemory};
    }

    if (hresult == E_UNEXPECTED) {
      return std::unexpected{ComputeMeshletsError::kUnexpected};
    }

    return std::unexpected{ComputeMeshletsError::kUnknown};
  };

  if (FAILED(hr)) {
    return to_error(hr);
  }

  auto dx_cull_data = std::vector<DirectX::CullData>{};
  dx_cull_data.resize(dx_meshlets.size());

  hr = DirectX::ComputeCullData(dx_positions.data(), dx_positions.size(), dx_meshlets.data(), dx_meshlets.size(),
    reinterpret_cast<IdxType*>(unique_vertex_indices.data()), unique_vertex_indices.size() / sizeof(IdxType),
    dx_primitives.data(), dx_primitives.size(), dx_cull_data.data());

  if (FAILED(hr)) {
    return to_error(hr);
  }

  auto result = ComputeMeshletsResult{};
  result.unique_vertex_indices = std::move(unique_vertex_indices);
  result.meshlets.reserve(dx_meshlets.size());
  result.primitive_indices.reserve(dx_primitives.size());
  result.cull_data.reserve(dx_cull_data.size());

  std::ranges::transform(dx_meshlets, std::back_inserter(result.meshlets), [](DirectX::Meshlet const& meshlet) {
    return MeshletData{
      .vert_count = meshlet.VertCount, .vert_offset = meshlet.VertOffset, .prim_count = meshlet.PrimCount,
      .prim_offset = meshlet.PrimOffset,
    };
  });

  std::ranges::transform(dx_primitives, std::back_inserter(result.primitive_indices),
    [](DirectX::MeshletTriangle const& tri) {
      return MeshletTriangleData{
        .idx0 = tri.i0, .idx1 = tri.i1, .idx2 = tri.i2,
      };
    });

  std::ranges::transform(dx_cull_data, std::back_inserter(result.cull_data),
    [](DirectX::CullData const& cull_data) {
      return MeshletCullData{
        .bounding_sphere = BoundingSphere{
          .center = Vector3{
            cull_data.BoundingSphere.Center.x, cull_data.BoundingSphere.Center.y, cull_data.BoundingSphere.Center.z
          },
          .radius = cull_data.BoundingSphere.Radius
        },
        .normal_cone = Vector<std::uint8_t, 4>{
          cull_data.NormalCone.x, cull_data.NormalCone.y,
          cull_data.NormalCone.z, cull_data.NormalCone.w
        },
        .apex_offset = cull_data.ApexOffset,
      };
    });


  return result;
}
}
