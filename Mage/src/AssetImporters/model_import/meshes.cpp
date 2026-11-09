#include "meshes.hpp"

#include <map>
#include <queue>

#include <spdlog/spdlog.h>

#include "scene.hpp"
#include "skinning.hpp"
#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/Serialization.hpp"


namespace sorcery::mage::model_import {
namespace {
// Extracts the vertex attributes, indices and material index of a mesh. Bone weights are left zeroed.
// Returns nullopt if the mesh is missing required vertex attributes.
[[nodiscard]]
auto ExtractSourceMesh(
  aiMesh const& mesh, unsigned idx_in_scene,
  std::filesystem::path const& src
) -> std::optional<SourceMesh> {
  // These meshes are always triangle-only, because
  // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines,
  // aiProcess_Triangulate splits up primitives with more than 3 vertices
  // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
  // TODO Implement non-triangle rendering support

  std::string const name{mesh.mName.Empty() ? std::format("Mesh {}", idx_in_scene) : mesh.mName.C_Str()};

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
}


auto CollectSourceMeshes(aiScene const& scene,
                         std::filesystem::path const& src) -> std::vector<std::optional<SourceMesh>> {
  std::vector<std::optional<SourceMesh>> meshes(scene.mNumMeshes);

  for (auto i = 0u; i < scene.mNumMeshes; ++i) {
    meshes[i] = ExtractSourceMesh(*scene.mMeshes[i], i, src);
  }

  return meshes;
}


auto CollectBakedMeshes(aiScene const& scene, std::filesystem::path const& src,
                        std::vector<SourceMesh>& out_meshes,
                        std::vector<SourceBone>& out_bone_proc_info) -> void {
  out_meshes.reserve(scene.mNumMeshes);

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
      auto const mesh = scene.mMeshes[mesh_scene_idx];

      auto mesh_proc_data = ExtractSourceMesh(*mesh, mesh_scene_idx, src);

      if (!mesh_proc_data) {
        continue;
      }

      for (auto j = 0uz; j < mesh_proc_data->positions.size(); ++j) {
        // Position, normal, and tangent lists are guaranteed to be same size

        auto& pos = mesh_proc_data->positions[j];
        pos = Vector3{Vector4{pos, 1} * abs_transform};

        auto& norm = mesh_proc_data->normals[j];
        norm = Vector3{Vector4{norm, 0} * abs_transform_inv_transp};

        auto& tangent = mesh_proc_data->tangents[j];
        tangent = Vector3{Vector4{tangent, 0} * abs_transform_inv_transp};
      }

      ExtractBoneWeights(*mesh, abs_transform_inv, out_bone_proc_info, *mesh_proc_data);
      out_meshes.emplace_back(std::move(*mesh_proc_data));
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      node_transform_queue.emplace(abs_transform, node->mChildren[i]);
    }

    node_transform_queue.pop();
  }
}


auto FuseByMaterial(std::span<SourceMesh const> const meshes) -> std::vector<SourceMesh> {
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
