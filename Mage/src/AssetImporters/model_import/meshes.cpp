#include "meshes.hpp"

#include <algorithm>
#include <concepts>
#include <map>
#include <queue>
#include <ranges>
#include <span>
#include <unordered_map>

#include <spdlog/spdlog.h>

#include "hash.hpp"
#include "identity.hpp"
#include "scene.hpp"
#include "skinning.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/mesh_utils.hpp"
#include "sorcery/Serialization.hpp"


namespace sorcery::mage::model_import {
namespace {
using NodesByMesh = std::vector<std::vector<NodeIndex>>;
using MeshesByGroup = std::vector<std::vector<MeshIndex>>;


[[nodiscard]]
auto CollectNodesByMesh(
  std::span<SceneNode const> const nodes,
  std::size_t const mesh_count
) -> NodesByMesh {
  NodesByMesh nodes_by_mesh(mesh_count);

  for (auto const [node_idx, node] : std::views::enumerate(nodes)) {
    for (auto j = 0u; j < node.ptr->mNumMeshes; ++j) {
      // Guard against the case where a node lists the same mesh multiple times
      if (auto& this_mesh_nodes = nodes_by_mesh[node.ptr->mMeshes[j]];
        this_mesh_nodes.empty() || std::cmp_not_equal(this_mesh_nodes.back(), node_idx)) {
        this_mesh_nodes.emplace_back(node_idx);
      }
    }
  }

  return nodes_by_mesh;
}


[[nodiscard]]
auto CollectMeshesByGroup(
  NodesByMesh const& nodes_by_mesh,
  std::span<std::optional<SourceMesh> const> const meshes
) -> MeshesByGroup {
  auto hash_node_list = [](std::span<NodeIndex const> const nodes) {
    auto h = Mix64(nodes.size());

    for (auto const n : nodes) {
      h = Mix64(h ^ n);
    }

    return h;
  };

  auto node_list_eq = [](std::span<NodeIndex const> const lhs, std::span<NodeIndex const> const rhs) {
    return std::ranges::equal(lhs, rhs);
  };

  std::unordered_map<std::span<NodeIndex const>, MeshGroupIndex, decltype(hash_node_list), decltype(node_list_eq)>
    groups_by_node_list;
  MeshesByGroup meshes_by_group;

  for (auto const [mesh_idx, node_list] : std::views::enumerate(nodes_by_mesh)) {
    // Meshes that failed to convert are just simply not added to any group
    // Saves us some extra bookkeeping later
    if (!meshes[mesh_idx]) {
      continue;
    }

    auto const [it, inserted] = groups_by_node_list.try_emplace(node_list, meshes_by_group.size());

    // If inserted, the group / node list pattern is first visited, so create a new group for it
    if (inserted) {
      meshes_by_group.emplace_back();
    }

    meshes_by_group[it->second].emplace_back(mesh_idx);
  }

  return meshes_by_group;
}


[[nodiscard]]
auto CollectGroupsByNode(
  MeshesByGroup const& meshes_by_group,
  NodesByMesh const& nodes_by_mesh,
  std::size_t const node_count
) -> GroupsByNode {
  GroupsByNode groups_by_node(node_count);

  for (auto const [group_idx, mesh_indices] : std::views::enumerate(meshes_by_group)) {
    // For each node, we generate the list of groups that the node references.
    // All meshes in the same group have the same node pattern so it doesnt matter which one we use, front is good.
    for (auto const node_idx : nodes_by_mesh[mesh_indices.front()]) {
      groups_by_node[node_idx].emplace_back(group_idx);
    }
  }

  return groups_by_node;
}


[[nodiscard]]
auto CollectAnchorNodeByGroup(
  NodesByMesh const& nodes_by_mesh,
  MeshesByGroup const& meshes_by_group
) -> AnchorNodeByGroup {
  return meshes_by_group | std::views::transform([&](auto const& meshes) -> std::optional<NodeIndex> {
    auto const first_mesh_idx = meshes.front();
    auto const& mesh_nodes = nodes_by_mesh.at(first_mesh_idx);

    if (mesh_nodes.empty()) {
      // Every unreferenced mesh is grouped into a single group,
      // so at most one group is unreferenced. This is for that case.
      return std::nullopt;
    }

    return mesh_nodes.front();
  }) | std::ranges::to<std::vector<std::optional<NodeIndex>>>();
}


// Extracts the vertex attributes, indices and material index of a mesh. Bone weights are left zeroed.
// Returns nullopt if the mesh is missing required vertex attributes.
[[nodiscard]]
auto ExtractSourceMesh(
  aiMesh const& mesh,
  unsigned const idx_in_scene,
  std::filesystem::path const& src
) -> std::optional<SourceMesh> {
  // These meshes are always triangle-only, because
  // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines,
  // aiProcess_Triangulate splits up primitives with more than 3 vertices
  // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
  // TODO Implement non-triangle rendering support

  auto const name{mesh.mName.Empty() ? std::format("Mesh {}", idx_in_scene) : mesh.mName.C_Str()};

  if (!mesh.HasPositions()) {
    spdlog::warn("Skipping mesh \"{}\" in file \"{}\" because it is missing vertex positions.", name,
      ToUntypedSv(src.u8string()));
    return std::nullopt;
  }

  if (!mesh.HasNormals()) {
    spdlog::warn("Skipping mesh \"{}\" in file \"{}\" because it is missing vertex normals.", name,
      ToUntypedSv(src.u8string()));
    return std::nullopt;
  }

  auto const has_uvs{mesh.HasTextureCoords(0)};

  if (!has_uvs) {
    spdlog::warn("Mesh \"{}\" in file \"{}\" is missing texture coordinates. Filling in zeroes.", name,
      ToUntypedSv(src.u8string()));
  }

  auto const has_tangents{mesh.HasTangentsAndBitangents()};

  if (!has_tangents) {
    spdlog::warn("Mesh \"{}\" in file \"{}\" is missing tangents. Filling in zeroes.", name,
      ToUntypedSv(src.u8string()));
  }

  SourceMesh ret;
  auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{ret};

  vertices.reserve(mesh.mNumVertices);
  normals.reserve(mesh.mNumVertices);
  uvs.reserve(mesh.mNumVertices);
  tangents.reserve(mesh.mNumVertices);
  bone_weights.resize(mesh.mNumVertices);
  bone_indices.resize(mesh.mNumVertices);

  for (unsigned j = 0; j < mesh.mNumVertices; j++) {
    vertices.emplace_back(Convert(mesh.mVertices[j]));
    normals.emplace_back(Normalized(Convert(mesh.mNormals[j])));
    tangents.emplace_back(has_tangents ? Convert(mesh.mTangents[j]) : Vector3{});
    uvs.emplace_back(has_uvs ? Vector2{Convert(mesh.mTextureCoords[0][j])} : Vector2{});
  }

  for (unsigned j = 0; j < mesh.mNumFaces; j++) {
    std::ranges::copy(std::span{mesh.mFaces[j].mIndices, mesh.mFaces[j].mNumIndices},
      std::back_inserter(indices));
  }

  mtlIdx = mesh.mMaterialIndex;

  return ret;
}


struct BakedMeshes {
  std::vector<SourceMesh> meshes;
  std::vector<SourceBone> bones; // the meshes' bone_indices index into this
};


// Collects every mesh of the scene with its node's absolute transform baked into its vertices,
// along with the bones referenced by the meshes
[[nodiscard]]
auto CollectBakedMeshes(
  aiScene const& scene,
  std::filesystem::path const& src
) -> BakedMeshes {
  BakedMeshes baked;

  baked.meshes.reserve(scene.mNumMeshes);

  struct NodeAndAccumTrafo {
    Matrix4 absolute_parent_transform;
    aiNode const* node;
  };

  std::queue<NodeAndAccumTrafo> node_transform_queue;
  node_transform_queue.emplace(Matrix4::Identity(), scene.mRootNode);

  while (!node_transform_queue.empty()) {
    auto const& [absolute_parent_transform, node] = node_transform_queue.front();
    auto const abs_transform{Convert(node->mTransformation).Transpose() * absolute_parent_transform};
    auto const abs_transform_inv{abs_transform.Inverse()};
    auto const abs_transform_inv_transp{abs_transform_inv.Transpose()};

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
      auto const mesh_scene_idx = node->mMeshes[i];
      auto const ai_mesh = scene.mMeshes[mesh_scene_idx];

      auto mesh = ExtractSourceMesh(*ai_mesh, mesh_scene_idx, src);

      if (!mesh) {
        continue;
      }

      for (auto j = 0uz; j < mesh->positions.size(); ++j) {
        // Position, normal, and tangent lists are guaranteed to be same size

        auto& pos = mesh->positions[j];
        pos = Vector3{Vector4{pos, 1} * abs_transform};

        auto& norm = mesh->normals[j];
        norm = Vector3{Vector4{norm, 0} * abs_transform_inv_transp};

        auto& tangent = mesh->tangents[j];
        tangent = Vector3{Vector4{tangent, 0} * abs_transform_inv_transp};
      }

      ExtractBoneWeights(*ai_mesh, abs_transform_inv, baked.bones, *mesh);
      baked.meshes.emplace_back(std::move(*mesh));
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      node_transform_queue.emplace(abs_transform, node->mChildren[i]);
    }

    node_transform_queue.pop();
  }

  return baked;
}


// Collects every mesh of the scene without any modification
[[nodiscard]]
auto CollectSourceMeshes(
  aiScene const& scene,
  std::filesystem::path const& src
) -> std::vector<std::optional<SourceMesh>> {
  std::vector<std::optional<SourceMesh>> meshes(scene.mNumMeshes);

  for (auto i = 0u; i < scene.mNumMeshes; ++i) {
    meshes[i] = ExtractSourceMesh(*scene.mMeshes[i], i, src);
  }

  return meshes;
}


// Fuses meshes that share materials
[[nodiscard]]
auto FuseByMaterial(
  std::span<SourceMesh const> const meshes
) -> std::vector<SourceMesh> {
  std::map<std::uint32_t, std::vector<SourceMesh const*>> meshes_per_mtl_idx;

  for (auto const& mesh : meshes) {
    meshes_per_mtl_idx[mesh.mtl_idx].push_back(&mesh);
  }

  std::vector<SourceMesh> fused_meshes;
  fused_meshes.reserve(meshes_per_mtl_idx.size());

  for (auto const& [mtl_idx, mtl_meshes] : meshes_per_mtl_idx) {
    auto& fused_mesh{fused_meshes.emplace_back()};

    fused_mesh.mtl_idx = mtl_idx;

    for (auto const& mesh : mtl_meshes) {
      // Bake 'baseVertex' into indices
      fused_mesh.indices.reserve(fused_mesh.indices.size() + mesh->indices.size());
      std::ranges::transform(mesh->indices, std::back_inserter(fused_mesh.indices),
        [&fused_mesh](unsigned const idx) {
          return static_cast<unsigned>(idx + fused_mesh.positions.size());
        });

      fused_mesh.positions.reserve(fused_mesh.positions.size() + mesh->positions.size());
      std::ranges::copy(mesh->positions, std::back_inserter(fused_mesh.positions));

      fused_mesh.normals.reserve(fused_mesh.normals.size() + mesh->normals.size());
      std::ranges::copy(mesh->normals, std::back_inserter(fused_mesh.normals));

      fused_mesh.tangents.reserve(fused_mesh.tangents.size() + mesh->tangents.size());
      std::ranges::copy(mesh->tangents, std::back_inserter(fused_mesh.tangents));

      fused_mesh.uvs.reserve(fused_mesh.uvs.size() + mesh->uvs.size());
      std::ranges::copy(mesh->uvs, std::back_inserter(fused_mesh.uvs));

      fused_mesh.bone_weights.reserve(fused_mesh.bone_weights.size() + mesh->bone_weights.size());
      std::ranges::copy(mesh->bone_weights, std::back_inserter(fused_mesh.bone_weights));

      fused_mesh.bone_indices.reserve(fused_mesh.bone_indices.size() + mesh->bone_indices.size());
      std::ranges::copy(mesh->bone_indices, std::back_inserter(fused_mesh.bone_indices));
    }
  }

  return fused_meshes;
}


template<class R, class T>
concept SizedRangeOf =
  std::ranges::forward_range<R> &&
  std::ranges::sized_range<R> &&
  std::same_as<std::ranges::range_const_reference_t<R>, T const&>;


// Meshletizes the meshes and appends them to the mesh data as submeshes, one per mesh.
// Fills in the geometry, the submeshes, the bounds and the index format.
[[nodiscard]]
auto BuildMeshGeometry(
  SizedRangeOf<SourceMesh const> auto&& meshes,
  bool const force_idx32,
  JobSystem& job_system,
  MeshData& mesh_data
) -> bool {
  // Determine index format

  mesh_data.idx32 = force_idx32 || std::ranges::any_of(meshes,
                      [](SourceMesh const& mesh) {
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


auto ImportBakedMesh(aiScene const& scene, std::filesystem::path const& src, MeshImportSettings const& settings,
                     JobSystem& job_system) -> std::optional<MeshImport> {
  auto [meshes, bones] = CollectBakedMeshes(scene, src);

  MeshImport mesh_import;
  auto& mesh_data = mesh_import.meshes.emplace_back();

  meshes = FuseByMaterial(meshes);

  if (!BuildMeshGeometry(meshes, settings.force_idx32, job_system, mesh_data)) {
    return std::nullopt;
  }

  auto skeleton = BuildSkeleton(*scene.mRootNode, bones);
  mesh_data.animations = CollectAnimations(scene, skeleton.node_idx_by_name);
  mesh_data.bones = std::move(skeleton.bones);
  mesh_data.skeleton = std::move(skeleton.nodes);

  mesh_import.labels.keys.emplace_back("mesh:baked");
  mesh_import.labels.names.emplace_back(ToUntypedSv(src.stem().u8string()));

  return mesh_import;
}


auto ImportMeshGroups(aiScene const& scene, std::filesystem::path const& src, MeshImportSettings const& settings,
                      JobSystem& job_system) -> std::optional<MeshImport> {
  auto const meshes = CollectSourceMeshes(scene, src);

  MeshImport mesh_import;
  auto& [nodes, groups_by_node] = mesh_import.hierarchy.emplace();
  nodes = FlattenNodeHierarchy(scene);

  auto const nodes_by_mesh = CollectNodesByMesh(nodes, meshes.size());
  auto const meshes_by_group = CollectMeshesByGroup(nodes_by_mesh, meshes);
  groups_by_node = CollectGroupsByNode(meshes_by_group, nodes_by_mesh, nodes.size());

  for (auto const& mesh_indices : meshes_by_group) {
    auto const group_meshes = mesh_indices | std::views::transform([&meshes](auto const mesh_idx) -> auto const& {
      return meshes[mesh_idx].value();
    });

    MeshData mesh_data;

    if (!BuildMeshGeometry(group_meshes, settings.force_idx32, job_system, mesh_data)) {
      return std::nullopt;
    }

    mesh_import.meshes.emplace_back(std::move(mesh_data));
  }

  auto const anchor_node_by_group = CollectAnchorNodeByGroup(nodes_by_mesh, meshes_by_group);
  mesh_import.labels = MakeMeshGroupLabels(nodes, anchor_node_by_group, src);
  return mesh_import;
}
}


auto ImportMeshes(aiScene const& scene, std::filesystem::path const& src,
                  MeshImportSettings const& settings, JobSystem& job_system) -> std::optional<MeshImport> {
  auto const has_bones = std::ranges::any_of(std::span{scene.mMeshes, scene.mNumMeshes},
    [](auto const* const mesh) { return mesh->HasBones(); });

  if (has_bones || settings.import_as_single_mesh) {
    return ImportBakedMesh(scene, src, settings, job_system);
  }

  return ImportMeshGroups(scene, src, settings, job_system);
}


auto SerializeMesh(MeshData const& mesh_data) -> std::vector<std::byte> {
  std::vector<std::byte> bytes;

  // Element counts

  SerializeToBinary(mesh_data.positions.size(), bytes);
  SerializeToBinary(mesh_data.meshlets.size(), bytes);
  SerializeToBinary(mesh_data.vertex_indices.size(), bytes);
  SerializeToBinary(mesh_data.triangle_indices.size(), bytes);
  SerializeToBinary(mesh_data.material_slots.size(), bytes);
  SerializeToBinary(mesh_data.submeshes.size(), bytes);
  SerializeToBinary(mesh_data.animations.size(), bytes);
  SerializeToBinary(mesh_data.skeleton.size(), bytes);
  SerializeToBinary(mesh_data.bones.size(), bytes);

  // Geometry data

  auto const pos_bytes{as_bytes(std::span{mesh_data.positions})};
  auto const norm_bytes{as_bytes(std::span{mesh_data.normals})};
  auto const tan_bytes{as_bytes(std::span{mesh_data.tangents})};
  auto const uv_bytes{as_bytes(std::span{mesh_data.uvs})};
  auto const bone_weight_bytes{as_bytes(std::span{mesh_data.bone_weights})};
  auto const bone_idx_bytes{as_bytes(std::span{mesh_data.bone_indices})};
  auto const meshlet_bytes{as_bytes(std::span{mesh_data.meshlets})};
  auto const vtx_idx_bytes{as_bytes(std::span{mesh_data.vertex_indices})};
  auto const prim_idx_bytes{as_bytes(std::span{mesh_data.triangle_indices})};
  auto const cull_data_bytes{as_bytes(std::span{mesh_data.cull_data})};

  bytes.reserve(
    std::size(bytes) + std::size(pos_bytes) + std::size(norm_bytes) + std::size(tan_bytes) + std::size(uv_bytes) +
    std::size(bone_weight_bytes) + std::size(bone_idx_bytes) + std::size(meshlet_bytes) + std::size(vtx_idx_bytes) +
    std::size(prim_idx_bytes) + std::size(cull_data_bytes));

  std::ranges::copy(pos_bytes, std::back_inserter(bytes));
  std::ranges::copy(norm_bytes, std::back_inserter(bytes));
  std::ranges::copy(tan_bytes, std::back_inserter(bytes));
  std::ranges::copy(uv_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_weight_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(meshlet_bytes, std::back_inserter(bytes));
  std::ranges::copy(vtx_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(prim_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(cull_data_bytes, std::back_inserter(bytes));

  // Material slots

  for (auto const& mtl_slot : mesh_data.material_slots) {
    SerializeToBinary(mtl_slot.name, bytes);
  }

  // Submeshes

  for (auto const& submesh : mesh_data.submeshes) {
    SerializeToBinary(submesh.first_meshlet, bytes);
    SerializeToBinary(submesh.meshlet_count, bytes);
    SerializeToBinary(submesh.base_vertex, bytes);
    SerializeToBinary(submesh.material_idx, bytes);

    for (auto i{0}; i < 3; i++) {
      SerializeToBinary(submesh.bounds.min[i], bytes);
    }

    for (auto i{0}; i < 3; i++) {
      SerializeToBinary(submesh.bounds.max[i], bytes);
    }
  }

  // Animations

  for (auto const& [name, duration, ticks_per_second, node_anims] : mesh_data.animations) {
    SerializeToBinary(name, bytes);
    SerializeToBinary(duration, bytes);
    SerializeToBinary(ticks_per_second, bytes);
    SerializeToBinary(node_anims.size(), bytes);

    for (auto const& [position_keys, rotation_keys, scaling_keys, node_idx] : node_anims) {
      SerializeToBinary(node_idx, bytes);

      SerializeToBinary(position_keys.size(), bytes);
      SerializeToBinary(rotation_keys.size(), bytes);
      SerializeToBinary(scaling_keys.size(), bytes);

      auto const pos_key_bytes{as_bytes(std::span{position_keys})};
      auto const rot_key_bytes{as_bytes(std::span{rotation_keys})};
      auto const scale_key_bytes{as_bytes(std::span{scaling_keys})};

      bytes.reserve(pos_key_bytes.size() + rot_key_bytes.size() + scale_key_bytes.size());

      std::ranges::copy(pos_key_bytes, std::back_inserter(bytes));
      std::ranges::copy(rot_key_bytes, std::back_inserter(bytes));
      std::ranges::copy(scale_key_bytes, std::back_inserter(bytes));
    }
  }

  // Skeleton nodes

  for (auto const& [name, transform, parent_idx] : mesh_data.skeleton) {
    SerializeToBinary(name, bytes);
    SerializeToBinary(parent_idx.has_value(), bytes);

    if (parent_idx) {
      SerializeToBinary(*parent_idx, bytes);
    }

    std::ranges::copy(as_bytes(std::span{transform.GetData(), 16}), std::back_inserter(bytes));
  }

  // Bones

  for (auto const& [offset_matrix, node_idx] : mesh_data.bones) {
    std::ranges::copy(as_bytes(std::span{offset_matrix.GetData(), 16}), std::back_inserter(bytes));
    SerializeToBinary(node_idx, bytes);
  }

  // Bounds

  for (auto i{0}; i < 3; i++) {
    SerializeToBinary(mesh_data.bounds.min[i], bytes);
  }

  for (auto i{0}; i < 3; i++) {
    SerializeToBinary(mesh_data.bounds.max[i], bytes);
  }

  // Index format

  SerializeToBinary(mesh_data.idx32, bytes);

  return bytes;
}
}
