#include "MeshImporter.hpp"

#include <algorithm>
#include <bit>
#include <cassert>
#include <format>
#include <iterator>
#include <limits>
#include <optional>
#include <queue>
#include <ranges>
#include <string_view>
#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Serialization.hpp"
#include "../EditorApp.hpp"
#include "../FileIo.hpp"
#include "../Resources/Mesh.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshImporter>{"Mesh Importer"}
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR
    .property("Fuse submeshes", &sorcery::MeshImporter::fuse_submeshes_);
}


namespace sorcery {
struct Node {
  std::string name;
  Matrix4 transform;
  Node* parent{nullptr};
  std::vector<std::unique_ptr<Node>> children;
  bool visited{false};
};


namespace {
[[nodiscard]] auto Convert(aiVector3D const& ai_vec) noexcept -> Vector3 {
  return Vector3{ai_vec.x, ai_vec.y, ai_vec.z};
}


[[nodiscard]] auto Convert(aiQuaternion const& ai_quat) noexcept -> Quaternion {
  return Quaternion{ai_quat.w, ai_quat.x, ai_quat.y, ai_quat.z};
}


[[nodiscard]] auto Convert(aiMatrix4x4 const& ai_mat) noexcept -> Matrix4 {
  return Matrix4{
    ai_mat.a1, ai_mat.a2, ai_mat.a3, ai_mat.a4,
    ai_mat.b1, ai_mat.b2, ai_mat.b3, ai_mat.b4,
    ai_mat.c1, ai_mat.c2, ai_mat.c3, ai_mat.c4,
    ai_mat.d1, ai_mat.d2, ai_mat.d3, ai_mat.d4
  };
}


[[nodiscard]] auto Convert(aiNode const* node, Node* const converted_parent) -> std::unique_ptr<Node> {
  auto converted_node{
    std::make_unique<Node>(node->mName.C_Str(), Convert(node->mTransformation).Transpose(), converted_parent)
  };

  for (unsigned i{0}; i < node->mNumChildren; i++) {
    converted_node->children.emplace_back(Convert(node->mChildren[i], converted_node.get()));
  }

  return converted_node;
}


[[nodiscard]] auto FindInHierarchyByName(Node& root_node, std::string_view const name) -> Node* {
  if (root_node.name == name) {
    return &root_node;
  }

  for (auto const& child : root_node.children) {
    if (auto const found{FindInHierarchyByName(*child, name)}) {
      return found;
    }
  }

  return nullptr;
}
}


auto MeshImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  std::string extensions;
  Assimp::Importer const importer;
  importer.GetExtensionList(extensions);

  // Assimp extension list format is "*.3ds;*.obj;*.dae"
  for (auto const ext : std::views::split(extensions, std::string_view{";"})) {
    out.emplace_back(std::string_view{std::begin(ext), std::end(ext)}.substr(1));
  }
}


auto MeshImporter::Import(std::filesystem::path const& src, std::vector<std::byte>& bytes,
                          ExternalResourceCategory& categ) -> bool {
  std::vector<unsigned char> meshBytes;

  if (!ReadFileBinary(src, meshBytes)) {
    return false;
  }

  Assimp::Importer importer;

  // We don't need these scene objects
  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  // We don't want to bother with non-triangle primitives
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  //importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);

  auto const scene{
    importer.ReadFileFromMemory(meshBytes.data(), meshBytes.size(),
      aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_TransformUVCoords |
      aiProcess_RemoveComponent, src.extension().string().c_str())
  };

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw std::runtime_error{std::format("Failed to import model at {}: {}.", src.string(), importer.GetErrorString())};
  }

  MeshData mesh_data;

  // Collect material info

  mesh_data.material_slots.resize(scene->mNumMaterials);

  for (unsigned i{0}; i < scene->mNumMaterials; i++) {
    mesh_data.material_slots[i].name = scene->mMaterials[i]->GetName().C_Str();
  }

  // Collect mesh info

  struct MeshProcessingData {
    std::vector<Vector3> positions;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<unsigned> indices;
    std::vector<Vector4> bone_weights;
    std::vector<Vector<std::uint32_t, 4>> bone_indices;
    unsigned mtl_idx{};
  };

  std::vector<MeshProcessingData> meshes;
  meshes.reserve(scene->mNumMeshes);

  struct BoneProcessingInfo {
    Matrix4 offset_matrix;
    std::string node_name;
  };

  std::vector<BoneProcessingInfo> bone_proc_info;

  struct NodeAndAccumTrafo {
    Matrix4 absolute_parent_transform;
    aiNode const* node;
  };

  std::queue<NodeAndAccumTrafo> node_transform_queue;
  node_transform_queue.emplace(Matrix4::Identity(), scene->mRootNode);

  while (!node_transform_queue.empty()) {
    auto const& [absolute_parent_transform, node] = node_transform_queue.front();
    auto const abs_transform{Convert(node->mTransformation).Transpose() * absolute_parent_transform};
    auto const abs_transform_inv{abs_transform.Inverse()};
    auto const abs_transform_inv_transp{abs_transform_inv.Transpose()};

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
      auto const mesh{scene->mMeshes[node->mMeshes[i]]};

      // These meshes are always triangle-only, because
      // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines, 
      // aiProcess_Triangulate splits up primitives with more than 3 vertices
      // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
      // TODO Implement non-triangle rendering support

      if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasTextureCoords(0) || !mesh->
          HasTangentsAndBitangents()) {
        std::string missing_attrs_list;

        if (!mesh->HasPositions()) {
          missing_attrs_list += "positions, ";
        }

        if (!mesh->HasNormals()) {
          missing_attrs_list += "normals, ";
        }

        if (!mesh->HasTextureCoords(0)) {
          missing_attrs_list += "texture coordinates, ";
        }

        if (!mesh->HasTangentsAndBitangents()) {
          missing_attrs_list += "tangents, ";
        }

        missing_attrs_list.erase(missing_attrs_list.size() - 2);

        OutputDebugStringA(std::format(
          "Skipping mesh {} in node \"{}\" of file \"{}\" because it is missing some attributes. The following attributes are missing: {}.\n",
          i, node->mName.C_Str(), src.string(), missing_attrs_list).c_str());
        continue;
      }

      auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{
        meshes.emplace_back()
      };

      vertices.reserve(mesh->mNumVertices);
      normals.reserve(mesh->mNumVertices);
      uvs.reserve(mesh->mNumVertices);
      tangents.reserve(mesh->mNumVertices);
      bone_weights.resize(mesh->mNumVertices);
      bone_indices.resize(mesh->mNumVertices);

      for (unsigned j = 0; j < mesh->mNumVertices; j++) {
        vertices.emplace_back(Vector4{Convert(mesh->mVertices[j]), 1} * abs_transform);
        normals.emplace_back(Vector4{Normalized(Convert(mesh->mNormals[j])), 0} * abs_transform_inv_transp);
        tangents.emplace_back(Vector4{Convert(mesh->mTangents[j]), 0} * abs_transform_inv_transp);
        uvs.emplace_back(mesh->HasTextureCoords(0) ? Vector2{Convert(mesh->mTextureCoords[0][j])} : Vector2{});
      }

      for (unsigned j = 0; j < mesh->mNumFaces; j++) {
        std::ranges::copy(std::span{mesh->mFaces[j].mIndices, mesh->mFaces[j].mNumIndices},
          std::back_inserter(indices));
      }

      mtlIdx = mesh->mMaterialIndex;

      for (unsigned j{0}; j < mesh->mNumBones; j++) {
        auto const bone{mesh->mBones[j]};

        auto const bone_idx{static_cast<std::uint32_t>(bone_proc_info.size())};
        bone_proc_info.emplace_back(abs_transform_inv * Convert(bone->mOffsetMatrix).Transpose(), bone->mName.C_Str());

        for (unsigned k{0}; k < bone->mNumWeights; k++) {
          auto const& weight{bone->mWeights[k]};

          auto found_free_weight_slot{false};

          for (auto l{0}; l < 4; l++) {
            if (bone_weights[weight.mVertexId][l] == 0.0f) {
              bone_weights[weight.mVertexId][l] = weight.mWeight;
              bone_indices[weight.mVertexId][l] = bone_idx;
              found_free_weight_slot = true;
              break;
            }
          }

          assert(found_free_weight_slot);
        }
      }
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      node_transform_queue.emplace(abs_transform, node->mChildren[i]);
    }

    node_transform_queue.pop();
  }

  // Convert node hierarchy

  auto root_node{Convert(scene->mRootNode, nullptr)};

  // Mark nodes that are part of the skeleton (but not necessarily bones!)

  std::ranges::for_each(bone_proc_info, [&root_node](BoneProcessingInfo const& bone_info) {
    // We look for the node corresponding to the bone and mark it along with all of its ancestors
    auto node{FindInHierarchyByName(*root_node, bone_info.node_name)};
    assert(node);

    while (node) {
      node->visited = true;
      node = node->parent;
    }
  });

  // Flatten visited hierarchy to a BFS list

  // Indices are needed for node animations. It is guaranteed that animated nodes have unique names.
  std::unordered_map<std::string, std::uint32_t> skeleton_node_name_to_idx;

  struct NodeAndParentIdx {
    Node* node;
    std::optional<std::uint32_t> parent_idx;
  };

  std::queue<NodeAndParentIdx> node_queue;

  if (!root_node->visited) {
    root_node.reset();
  } else {
    node_queue.emplace(root_node.get(), std::nullopt);
  }

  while (!node_queue.empty()) {
    auto const& [node, parent_idx]{node_queue.front()};
    auto const this_node_idx{static_cast<std::uint32_t>(mesh_data.skeleton.size())};
    mesh_data.skeleton.emplace_back(node->name, node->transform, parent_idx);
    skeleton_node_name_to_idx.try_emplace(node->name, this_node_idx);

    for (auto const& child : node->children) {
      if (child->visited) {
        node_queue.emplace(child.get(), this_node_idx);
      }
    }

    node_queue.pop();
  }

  // Create final bone data

  mesh_data.bones.reserve(bone_proc_info.size());
  std::ranges::transform(bone_proc_info, std::back_inserter(mesh_data.bones),
    [&skeleton_node_name_to_idx](BoneProcessingInfo const& bone_info) {
      return Bone{
        .offset_mtx = bone_info.offset_matrix, .skeleton_node_idx = skeleton_node_name_to_idx.at(bone_info.node_name)
      };
    });

  if (fuse_submeshes_) {
    // Fuse meshes that share materials

    std::map<std::uint32_t, std::vector<MeshProcessingData const*>> meshes_per_mtl_idx;

    for (auto const& mesh : meshes) {
      meshes_per_mtl_idx[mesh.mtl_idx].push_back(&mesh);
    }

    std::vector<MeshProcessingData> fused_meshes;
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

    meshes = std::move(fused_meshes);
  }

  // Determine index format

  mesh_data.idx32 = std::ranges::any_of(meshes, [](MeshProcessingData const& mesh) {
    return std::ranges::any_of(mesh.indices, [](unsigned const idx) {
      return idx > std::numeric_limits<std::uint16_t>::max();
    });
  });

  // Meshletize submeshes

  struct MeshletizedMesh {
    std::vector<MeshletData> meshlets;
    std::vector<std::uint8_t> unique_vertex_indices;
    std::vector<MeshletTriangleData> primitive_indices;
  };

  std::vector<std::optional<MeshletizedMesh>> meshletized_meshes(meshes.size());
  std::vector<ObserverPtr<Job>> meshletize_jobs;
  meshletize_jobs.reserve(meshes.size());

  for (std::size_t i{0}; i < meshes.size(); i++) {
    meshletize_jobs.emplace_back(App::Instance().GetJobSystem().CreateJob(
      [i, &meshes, &meshletized_meshes, idx32 = mesh_data.idx32] {
        auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtl_idx]{meshes[i]};
        auto [meshlets, unique_vertex_indices, primitive_indices]{MeshletizedMesh{}};

        bool success;

        if (idx32) {
          success = SUCCEEDED(ComputeMeshlets(indices, positions, meshlets, unique_vertex_indices, primitive_indices));
        } else {
          std::vector<std::uint16_t> indices16(indices.size());
          std::ranges::transform(indices, indices16.begin(), [](unsigned const idx) {
            return static_cast<std::uint16_t>(idx);
          });

          success = SUCCEEDED(
            ComputeMeshlets(indices16, positions, meshlets, unique_vertex_indices, primitive_indices));
        }

        if (success) {
          meshletized_meshes[i] = MeshletizedMesh{
            std::move(meshlets), std::move(unique_vertex_indices),
            std::move(primitive_indices)
          };
        }
      }));

    App::Instance().GetJobSystem().Run(meshletize_jobs.back());
  }

  for (auto const job : meshletize_jobs) {
    App::Instance().GetJobSystem().Wait(job);
  }

  // Combine geometry

  std::vector<SubmeshMeshletRange> submesh_meshlet_ranges;
  submesh_meshlet_ranges.reserve(meshes.size());

  std::vector<AABB> submesh_bounds;
  submesh_bounds.reserve(meshes.size());

  std::vector<std::uint32_t> submesh_base_vertices;
  submesh_base_vertices.reserve(meshes.size());

  for (std::size_t i{0}; i < meshes.size(); i++) {
    if (!meshletized_meshes[i]) {
      throw std::runtime_error{"Failed to compute meshlets."};
    }

    auto& [meshlets, unique_vertex_indices, primitive_indices]{*meshletized_meshes[i]};
    auto& [positions, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{meshes[i]};

    submesh_base_vertices.emplace_back(static_cast<std::uint32_t>(mesh_data.positions.size()));

    submesh_meshlet_ranges.emplace_back(mesh_data.meshlets.size(), meshlets.size());

    mesh_data.meshlets.reserve(mesh_data.meshlets.size() + meshlets.size());
    std::ranges::transform(meshlets, std::back_inserter(mesh_data.meshlets),
      [&mesh_data](MeshletData const& meshlet) {
        // Offset index ranges by existing index counts
        return MeshletData{
          .vert_count = meshlet.vert_count,
          .vert_offset = meshlet.vert_offset + static_cast<std::uint32_t>(mesh_data.vertex_indices.size() / 4),
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
  }

  // Create and store submeshes

  mesh_data.submeshes.reserve(meshes.size());
  for (std::size_t i{0}; i < meshes.size(); i++) {
    mesh_data.submeshes.emplace_back(SubmeshData{
      .first_meshlet = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].first_meshlet),
      .meshlet_count = static_cast<std::uint32_t>(submesh_meshlet_ranges[i].meshlet_count),
      .base_vertex = submesh_base_vertices[i],
      .material_idx = meshes[i].mtl_idx,
      .bounds = submesh_bounds[i],
    });
  }

  // Collect animations

  mesh_data.animations.reserve(scene->mNumAnimations);

  for (unsigned i{0}; i < scene->mNumAnimations; i++) {
    auto const anim{scene->mAnimations[i]};

    std::vector<NodeAnimation> node_anims;

    for (unsigned j{0}; j < anim->mNumChannels; j++) {
      auto const channel{anim->mChannels[j]};

      // Check if the animated node is an ancestor of a mesh's node

      std::queue<aiNode const*> node_search_queue;
      node_search_queue.emplace(scene->mRootNode);

      std::queue<aiNode const*> mesh_search_queue;

      // Find the node that the channel is animating
      while (!node_search_queue.empty()) {
        auto const node{node_search_queue.front()};

        if (node->mName == channel->mNodeName) {
          mesh_search_queue.emplace(node);
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          node_search_queue.emplace(node->mChildren[k]);
        }

        node_search_queue.pop();
      }

      // Check if any of the node's descendants are mesh nodes
      while (!mesh_search_queue.empty()) {
        auto const node{mesh_search_queue.front()};

        if (node->mNumMeshes > 0) {
          // TODO log this properly
          OutputDebugStringA(
            "Animated node is an ancestor of a submesh node. Animations on submesh nodes are currently not supported.\n");
          break;
        }

        for (unsigned k{0}; k < node->mNumChildren; k++) {
          mesh_search_queue.emplace(node->mChildren[k]);
        }

        mesh_search_queue.pop();
      }

      // Check if the animated node is part of the skeleton

      if (!skeleton_node_name_to_idx.contains(channel->mNodeName.C_Str())) {
        // TODO log this properly
        OutputDebugStringA("Animated node is not part of the skeleton. Such animations are currently not supported.\n");
        continue;
      }

      std::vector<AnimPositionKey> position_keys;
      position_keys.reserve(channel->mNumPositionKeys);
      std::ranges::transform(channel->mPositionKeys, channel->mPositionKeys + channel->mNumPositionKeys,
        std::back_inserter(position_keys), [](aiVectorKey const& pos_key) {
          return AnimPositionKey{
            static_cast<float>(pos_key.mTime), Convert(pos_key.mValue)
          };
        });

      std::vector<AnimRotationKey> rotation_keys;
      rotation_keys.reserve(channel->mNumRotationKeys);
      std::ranges::transform(channel->mRotationKeys, channel->mRotationKeys + channel->mNumRotationKeys,
        std::back_inserter(rotation_keys), [](aiQuatKey const& rot_key) {
          return AnimRotationKey{
            static_cast<float>(rot_key.mTime), Convert(rot_key.mValue)
          };
        });

      std::vector<AnimScalingKey> scaling_keys;
      scaling_keys.reserve(channel->mNumScalingKeys);
      std::ranges::transform(channel->mScalingKeys, channel->mScalingKeys + channel->mNumScalingKeys,
        std::back_inserter(scaling_keys), [](aiVectorKey const& scale_key) {
          return AnimScalingKey{
            static_cast<float>(scale_key.mTime), Convert(scale_key.mValue)
          };
        });

      node_anims.emplace_back(std::move(position_keys), std::move(rotation_keys), std::move(scaling_keys),
        skeleton_node_name_to_idx[channel->mNodeName.C_Str()]);
    }

    mesh_data.animations.emplace_back(anim->mName.C_Str(), static_cast<float>(anim->mDuration),
      static_cast<float>(anim->mTicksPerSecond), std::move(node_anims));
  }

  // Bounds

  mesh_data.bounds = AABB::FromVertices(mesh_data.positions);

  // Serialize

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

  bytes.reserve(
    std::size(bytes) + std::size(pos_bytes) + std::size(norm_bytes) + std::size(tan_bytes) + std::size(uv_bytes) +
    std::size(bone_weight_bytes) + std::size(bone_idx_bytes) + std::size(meshlet_bytes) + std::size(vtx_idx_bytes) +
    std::size(prim_idx_bytes));

  std::ranges::copy(pos_bytes, std::back_inserter(bytes));
  std::ranges::copy(norm_bytes, std::back_inserter(bytes));
  std::ranges::copy(tan_bytes, std::back_inserter(bytes));
  std::ranges::copy(uv_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_weight_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(meshlet_bytes, std::back_inserter(bytes));
  std::ranges::copy(vtx_idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(prim_idx_bytes, std::back_inserter(bytes));

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

  categ = ExternalResourceCategory::Mesh;
  return true;
}


auto MeshImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  return rttr::type::get<Mesh>();
}
}
