#pragma once

#include <spdlog/spdlog.h>

#include "sorcery/mesh_utils.hpp"


namespace sorcery::mage::model_import {
auto BuildMeshGeometry(SizedRangeOf<MeshProcessingData const> auto&& meshes, bool const force_idx32,
                       JobSystem& job_system, MeshData& mesh_data) -> bool {
  // Determine index format

  mesh_data.idx32 = force_idx32 || std::ranges::any_of(meshes,
                      [](MeshProcessingData const& mesh) {
                        return std::ranges::any_of(mesh.indices, [](unsigned const idx) {
                          return idx > std::numeric_limits<std::uint16_t>::max();
                        });
                      });

  // Meshletize submeshes

  struct MeshletizedMesh {
    std::vector<MeshletData> meshlets;
    std::vector<std::uint8_t> unique_vertex_indices;
    std::vector<MeshletTriangleData> primitive_indices;
    std::vector<MeshletCullData> cull_data;
  };

  std::vector<std::optional<MeshletizedMesh>> meshletized_meshes(std::ranges::size(meshes));
  std::vector<ObserverPtr<Job>> meshletize_jobs;
  meshletize_jobs.reserve(std::ranges::size(meshes));

  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    meshletize_jobs.emplace_back(job_system.CreateJob(
      [i, &mesh, &meshletized_meshes, idx32 = mesh_data.idx32] {
        auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtl_idx]{mesh};
        auto [meshlets, unique_vertex_indices, primitive_indices, cull_data]{MeshletizedMesh{}};

        bool success;

        if (idx32) {
          success = ComputeMeshlets<std::uint32_t, Vector3>(indices, positions, meshlets, unique_vertex_indices,
            primitive_indices, cull_data);
        } else {
          std::vector<std::uint16_t> indices16(indices.size());
          std::ranges::transform(indices, indices16.begin(), [](unsigned const idx) {
            return static_cast<std::uint16_t>(idx);
          });

          success = ComputeMeshlets<std::uint16_t, Vector3>(indices16, positions, meshlets, unique_vertex_indices,
            primitive_indices, cull_data);
        }

        if (success) {
          meshletized_meshes[i] = MeshletizedMesh{
            std::move(meshlets), std::move(unique_vertex_indices),
            std::move(primitive_indices), std::move(cull_data)
          };
        }
      }));

    job_system.Submit(meshletize_jobs.back());
  }

  job_system.WaitAll(meshletize_jobs);

  // Combine geometry

  std::vector<SubmeshMeshletRange> submesh_meshlet_ranges;
  submesh_meshlet_ranges.reserve(std::ranges::size(meshes));

  std::vector<AABB> submesh_bounds;
  submesh_bounds.reserve(std::ranges::size(meshes));

  std::vector<std::uint32_t> submesh_base_vertices;
  submesh_base_vertices.reserve(std::ranges::size(meshes));

  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    if (!meshletized_meshes[i]) {
      spdlog::error("Failed to compute meshlets for submesh {}.", i);
      return false;
    }

    auto& [meshlets, unique_vertex_indices, primitive_indices, cull_data]{*meshletized_meshes[i]};
    auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{mesh};

    submesh_base_vertices.emplace_back(static_cast<std::uint32_t>(mesh_data.positions.size()));

    submesh_meshlet_ranges.emplace_back(mesh_data.meshlets.size(), meshlets.size());

    mesh_data.meshlets.reserve(mesh_data.meshlets.size() + meshlets.size());
    std::ranges::transform(meshlets, std::back_inserter(mesh_data.meshlets),
      [&mesh_data](MeshletData const& meshlet) {
        // Offset index ranges by existing index counts
        return MeshletData{
          .vert_count = meshlet.vert_count,
          .vert_offset = meshlet.vert_offset + static_cast<std::uint32_t>(
                           mesh_data.vertex_indices.size() / (mesh_data.idx32 ? 4 : 2)),
          .prim_count = meshlet.prim_count,
          .prim_offset = meshlet.prim_offset + static_cast<std::uint32_t>(mesh_data.triangle_indices.size()),
        };
      });

    submesh_bounds.emplace_back(AABB::FromVertices(positions));

    mesh_data.positions.reserve(mesh_data.positions.size() + positions.size());
    std::ranges::copy(positions, std::back_inserter(mesh_data.positions));

    mesh_data.normals.reserve(mesh_data.normals.size() + normals.size());
    std::ranges::copy(normals, std::back_inserter(mesh_data.normals));

    mesh_data.tangents.reserve(mesh_data.tangents.size() + tangents.size());
    std::ranges::copy(tangents, std::back_inserter(mesh_data.tangents));

    mesh_data.uvs.reserve(mesh_data.uvs.size() + uvs.size());
    std::ranges::copy(uvs, std::back_inserter(mesh_data.uvs));

    mesh_data.bone_weights.reserve(mesh_data.bone_weights.size() + bone_weights.size());
    std::ranges::copy(bone_weights, std::back_inserter(mesh_data.bone_weights));

    mesh_data.bone_indices.reserve(mesh_data.bone_indices.size() + bone_indices.size());
    std::ranges::copy(bone_indices, std::back_inserter(mesh_data.bone_indices));

    mesh_data.vertex_indices.reserve(mesh_data.vertex_indices.size() + unique_vertex_indices.size());
    std::ranges::copy(unique_vertex_indices, std::back_inserter(mesh_data.vertex_indices));

    mesh_data.triangle_indices.reserve(mesh_data.triangle_indices.size() + primitive_indices.size());
    std::ranges::copy(primitive_indices, std::back_inserter(mesh_data.triangle_indices));

    mesh_data.cull_data.reserve(mesh_data.cull_data.size() + cull_data.size());
    std::ranges::copy(cull_data, std::back_inserter(mesh_data.cull_data));
  }

  // Create and store submeshes

  mesh_data.submeshes.reserve(std::ranges::size(meshes));
  for (auto const [i, mesh] : std::views::enumerate(meshes)) {
    mesh_data.submeshes.emplace_back(SubmeshData{
      .first_meshlet = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].first_meshlet),
      .meshlet_count = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].meshlet_count),
      .base_vertex = submesh_base_vertices[i],
      .material_idx = mesh.mtl_idx,
      .bounds = submesh_bounds[i],
    });
  }

  // Bounds

  mesh_data.bounds = AABB::FromVertices(mesh_data.positions);

  return true;
}
}
