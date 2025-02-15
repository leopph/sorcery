#include "MeshImporter.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <limits>
#include <optional>
#include <queue>
#include <ranges>
#include <string_view>
#include <utility>

#include <DirectXMesh.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Serialization.hpp"
#include "../FileIo.hpp"
#include "../Resources/Mesh.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshImporter>{"Mesh Importer"}
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR;
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

  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);

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
    std::vector<Vector3> vertices;
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

      auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{
        meshes.emplace_back()
      };

      if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasTextureCoords(0) || !mesh->
          HasTangentsAndBitangents()) {
        // TODO log or something
        continue;
      }

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

  std::vector<SkeletonNode> skeleton_nodes;

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
    auto const this_node_idx{static_cast<std::uint32_t>(skeleton_nodes.size())};
    skeleton_nodes.emplace_back(node->name, node->transform, parent_idx);
    skeleton_node_name_to_idx.try_emplace(node->name, this_node_idx);

    for (auto const& child : node->children) {
      if (child->visited) {
        node_queue.emplace(child.get(), this_node_idx);
      }
    }

    node_queue.pop();
  }

  // Create final bone data

  std::vector<Bone> bones;
  bones.reserve(bone_proc_info.size());
  std::ranges::transform(bone_proc_info, std::back_inserter(bones),
    [&skeleton_node_name_to_idx](BoneProcessingInfo const& bone_info) {
      return Bone{
        .offset_mtx = bone_info.offset_matrix, .skeleton_node_idx = skeleton_node_name_to_idx.at(bone_info.node_name)
      };
    });

  // Store geometry data and create submeshes

  mesh_data.submeshes.reserve(std::size(meshes));

  for (auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx] : meshes) {
    std::vector<DirectX::XMFLOAT3> dx_vertices;
    dx_vertices.reserve(std::size(vertices));
    std::ranges::transform(vertices, std::back_inserter(dx_vertices), [](Vector3 const& v) {
      return DirectX::XMFLOAT3{v[0], v[1], v[2]};
    });

    std::vector<DirectX::Meshlet> dx_meshlets;
    std::vector<std::uint8_t> vertex_indices;
    std::vector<DirectX::MeshletTriangle> dx_primitive_indices;

    if (FAILED(
      ComputeMeshlets(indices.data(), indices.size() / 3, dx_vertices.data(), dx_vertices.size(), nullptr, dx_meshlets,
        vertex_indices, dx_primitive_indices, kMeshletMaxVerts, kMeshletMaxPrims))) {
      // TODO log this properly
      OutputDebugStringA("Failed to compute meshlets.\n");
      continue;
    }

    std::vector<MeshletData> meshlets;
    meshlets.reserve(std::size(dx_meshlets));
    std::ranges::transform(dx_meshlets, std::back_inserter(meshlets), [](DirectX::Meshlet const& meshlet) {
      return MeshletData{
        .vert_count = meshlet.VertCount, .prim_count = meshlet.PrimCount, .vert_offset = meshlet.VertOffset,
        .prim_offset = meshlet.PrimOffset,
      };
    });

    std::vector<MeshletTriangleIndexData> primitive_indices;
    primitive_indices.reserve(std::size(dx_primitive_indices));
    std::ranges::transform(dx_primitive_indices, std::back_inserter(primitive_indices),
      [](DirectX::MeshletTriangle const& tri) {
        return MeshletTriangleIndexData{
          .idx0 = tri.i0, .idx1 = tri.i1, .idx2 = tri.i2,
        };
      });

    mesh_data.submeshes.emplace_back(std::move(vertices), std::move(normals), std::move(tangents), std::move(uvs),
      std::move(bone_weights), std::move(bone_indices), std::move(meshlets), std::move(vertex_indices),
      std::move(primitive_indices), mtlIdx, true /*TODO convert to 16bit if possible*/);
  }

  // Collect animations

  std::vector<Animation> animations;
  animations.reserve(scene->mNumAnimations);

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

    animations.emplace_back(anim->mName.C_Str(), static_cast<float>(anim->mDuration),
      static_cast<float>(anim->mTicksPerSecond), std::move(node_anims));
  }

  // Serialize

  // Element counts

  SerializeToBinary(std::size(mesh_data.material_slots), bytes);
  SerializeToBinary(std::size(mesh_data.submeshes), bytes);
  SerializeToBinary(std::ssize(mesh_data.animations), bytes);
  SerializeToBinary(std::ssize(skeleton_nodes), bytes);
  SerializeToBinary(std::ssize(bones), bytes);

  // Material slots

  for (auto const& mtl_slot : mesh_data.material_slots) {
    SerializeToBinary(mtl_slot.name, bytes);
  }

  // Submeshes

  for (auto const& submesh : mesh_data.submeshes) {
    SerializeToBinary(submesh.positions.size(), bytes);
    SerializeToBinary(submesh.meshlets.size(), bytes);
    SerializeToBinary(submesh.vertex_indices.size(), bytes);
    SerializeToBinary(submesh.triangle_indices.size(), bytes);

    auto const pos_bytes{as_bytes(std::span{submesh.positions})};
    auto const norm_bytes{as_bytes(std::span{submesh.normals})};
    auto const tan_bytes{as_bytes(std::span{submesh.tangents})};
    auto const uv_bytes{as_bytes(std::span{submesh.uvs})};
    auto const bone_weight_bytes{as_bytes(std::span{submesh.bone_weights})};
    auto const bone_idx_bytes{as_bytes(std::span{submesh.bone_indices})};
    auto const meshlet_bytes{as_bytes(std::span{submesh.meshlets})};
    auto const vtx_idx_bytes{as_bytes(std::span{submesh.vertex_indices})};
    auto const prim_idx_bytes{as_bytes(std::span{submesh.triangle_indices})};

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
    SerializeToBinary(submesh.material_idx, bytes);
    SerializeToBinary(submesh.idx32, bytes);
  }

  // Animations

  for (auto const& [name, duration, ticks_per_second, node_anims] : animations) {
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

  for (auto const& [name, transform, parent_idx] : skeleton_nodes) {
    SerializeToBinary(name, bytes);
    SerializeToBinary(parent_idx.has_value(), bytes);

    if (parent_idx) {
      SerializeToBinary(*parent_idx, bytes);
    }

    std::ranges::copy(as_bytes(std::span{transform.GetData(), 16}), std::back_inserter(bytes));
  }

  // Bones

  for (auto const& [offset_matrix, node_idx] : bones) {
    std::ranges::copy(as_bytes(std::span{offset_matrix.GetData(), 16}), std::back_inserter(bytes));
    SerializeToBinary(node_idx, bytes);
  }

  categ = ExternalResourceCategory::Mesh;
  return true;
}


auto MeshImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  return rttr::type::get<Mesh>();
}
}
