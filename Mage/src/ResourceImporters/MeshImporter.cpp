#include "MeshImporter.hpp"

#include <algorithm>
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
      aiProcess_RemoveComponent)
  };

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw std::runtime_error{std::format("Failed to import model at {}: {}.", src.string(), importer.GetErrorString())};
  }

  Mesh::Data mesh_data;

  // Collect all info from aiMaterials

  mesh_data.material_slots.resize(scene->mNumMaterials);

  for (unsigned i{0}; i < scene->mNumMaterials; i++) {
    mesh_data.material_slots[i].name = scene->mMaterials[i]->GetName().C_Str();
  }

  // Collect all info from aiMeshes

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

  struct BoneProcessingInfo {
    Matrix4 offset_matrix;
    std::string node_name;
  };

  std::unordered_map<std::string, std::uint32_t> bone_name_to_idx;
  std::vector<BoneProcessingInfo> bone_proc_info;

  // Collected mesh data not yet transformed by node matrices
  std::vector<MeshProcessingData> meshes_untransformed;
  meshes_untransformed.reserve(scene->mNumMeshes);

  for (unsigned i = 0; i < scene->mNumMeshes; i++) {
    // These meshes are always triangle-only, because
    // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines, 
    // aiProcess_Triangulate splits up primitives with more than 3 vertices
    // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
    // TODO Implement non-triangle rendering support

    aiMesh const* const mesh{scene->mMeshes[i]};
    auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{
      meshes_untransformed.emplace_back()
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
      vertices.emplace_back(Convert(mesh->mVertices[j]));
      normals.emplace_back(Normalized(Convert(mesh->mNormals[j])));
      uvs.emplace_back(mesh->HasTextureCoords(0) ? Vector2{Convert(mesh->mTextureCoords[0][j])} : Vector2{});
      tangents.emplace_back(Convert(mesh->mTangents[j]));
    }

    for (unsigned j = 0; j < mesh->mNumFaces; j++) {
      std::ranges::copy(std::span{mesh->mFaces[j].mIndices, mesh->mFaces[j].mNumIndices}, std::back_inserter(indices));
    }

    mtlIdx = mesh->mMaterialIndex;

    for (unsigned j{0}; j < mesh->mNumBones; j++) {
      if (auto const bone_idx{static_cast<std::uint32_t>(bone_name_to_idx.size())}; bone_name_to_idx.try_emplace(
        mesh->mBones[j]->mName.C_Str(), bone_idx).second) {
        bone_proc_info.emplace_back(Convert(mesh->mBones[j]->mOffsetMatrix).Transpose(),
          mesh->mBones[j]->mName.C_Str());
      }

      for (unsigned k{0}; k < mesh->mBones[j]->mNumWeights; k++) {
        auto const& weight{mesh->mBones[j]->mWeights[k]};

        for (auto l{0}; l < 4; l++) {
          if (bone_weights[weight.mVertexId][l] == 0.0f) {
            bone_weights[weight.mVertexId][l] = weight.mWeight;
            bone_indices[weight.mVertexId][l] = bone_name_to_idx[mesh->mBones[j]->mName.C_Str()];
            break;
          }
        }
      }
    }
  }

  // Convert node hierarchy

  auto root_node{Convert(scene->mRootNode, nullptr)};

  // Mark nodes that are part of the skeleton (but not necessarily bones!)

  std::ranges::for_each(bone_proc_info, [&root_node](BoneProcessingInfo const& bone_info) {
    // If we find the node corresponding to the bone, mark it and all its parents
    if (auto node{FindInHierarchyByName(*root_node, bone_info.node_name)}) {
      while (node) {
        node->visited = true;
        node = node->parent;
      }
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
      return Bone{bone_info.offset_matrix, skeleton_node_name_to_idx.at(bone_info.node_name)};
    });

  // Accumulate node trafos

  struct MeshTrafoAndIndex {
    Matrix4 trafo;
    unsigned meshIdx;
  };

  struct NodeAndAccumTrafo {
    Matrix4 accumParentTrafo;
    aiNode const* node;
  };

  std::vector<MeshTrafoAndIndex> meshIndicesWithTrafos;
  std::queue<NodeAndAccumTrafo> transformQueue;
  transformQueue.emplace(Matrix4::Identity(), scene->mRootNode);

  while (!transformQueue.empty()) {
    auto const& [accumParentTrafo, node] = transformQueue.front();
    auto const trafo{Convert(node->mTransformation).Transpose() * accumParentTrafo};

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
      meshIndicesWithTrafos.emplace_back(trafo, node->mMeshes[i]);
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      transformQueue.emplace(trafo, node->mChildren[i]);
    }

    transformQueue.pop();
  }

  // Transform mesh geometry using trafos

  // Collected mesh data transformed by node matrices
  std::vector<MeshProcessingData> meshesTransformed;

  for (auto const& [trafo, meshIdx] : meshIndicesWithTrafos) {
    auto& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx]{
      meshesTransformed.emplace_back(meshes_untransformed[meshIdx])
    };

    Matrix4 const trafoInvTransp{trafo.Inverse().Transpose()};

    for (int i = 0; i < std::ssize(vertices); i++) {
      vertices[i] = Vector3{Vector4{vertices[i], 1} * trafo};
      normals[i] = Vector3{Vector4{normals[i], 0} * trafoInvTransp};
      tangents[i] = Vector3{Vector4{tangents[i], 0} * trafoInvTransp};
    }
  }

  // Store geometry data and create submeshes

  mesh_data.sub_meshes.reserve(std::size(meshesTransformed));
  mesh_data.indices.emplace<std::vector<std::uint32_t>>();

  for (auto const& [vertices, normals, uvs, tangents, indices, bone_weights, bone_indices, mtlIdx] :
       meshesTransformed) {
    mesh_data.sub_meshes.emplace_back(static_cast<int>(std::ssize(mesh_data.positions)),
      std::visit([]<typename T>(std::vector<T> const& indices) { return static_cast<int>(std::ssize(indices)); },
        mesh_data.indices), static_cast<int>(std::ssize(indices)), static_cast<int>(mtlIdx), AABB{});

    std::ranges::copy(vertices, std::back_inserter(mesh_data.positions));
    std::ranges::copy(normals, std::back_inserter(mesh_data.normals));
    std::ranges::copy(uvs, std::back_inserter(mesh_data.uvs));
    std::ranges::copy(tangents, std::back_inserter(mesh_data.tangents));
    std::ranges::copy(indices, std::back_inserter(std::get<std::vector<std::uint32_t>>(mesh_data.indices)));
    std::ranges::copy(bone_weights, std::back_inserter(mesh_data.bone_weights));
    std::ranges::copy(bone_indices, std::back_inserter(mesh_data.bone_indices));
  }

  // Transform indices to 16-bit if possible

  if (auto const& indices32{std::get<std::vector<std::uint32_t>>(mesh_data.indices)}; std::ranges::all_of(indices32,
    [](std::uint32_t const idx) {
      return idx <= std::numeric_limits<std::uint16_t>::max();
    })) {
    std::vector<std::uint16_t> indices16;
    indices16.reserve(std::size(indices32));

    std::ranges::transform(indices32, std::back_inserter(indices16), [](std::uint32_t const idx) {
      return static_cast<std::uint16_t>(idx);
    });

    mesh_data.indices = std::move(indices16);
  }

  // Collect animations

  std::vector<Animation> animations;
  animations.reserve(scene->mNumAnimations);

  for (unsigned i{0}; i < scene->mNumAnimations; i++) {
    auto const ai_anim{scene->mAnimations[i]};

    std::vector<NodeAnimation> node_anims;

    for (unsigned j{0}; j < scene->mAnimations[i]->mNumChannels; j++) {
      auto const ai_channel{scene->mAnimations[i]->mChannels[j]};

      std::vector<PositionKey> position_keys;
      position_keys.reserve(ai_channel->mNumPositionKeys);
      std::ranges::transform(ai_channel->mPositionKeys, ai_channel->mPositionKeys + ai_channel->mNumPositionKeys,
        std::back_inserter(position_keys), [](aiVectorKey const& pos_key) {
          return PositionKey{
            static_cast<float>(pos_key.mTime), Convert(pos_key.mValue)
          };
        });

      std::vector<RotationKey> rotation_keys;
      rotation_keys.reserve(ai_channel->mNumRotationKeys);
      std::ranges::transform(ai_channel->mRotationKeys, ai_channel->mRotationKeys + ai_channel->mNumRotationKeys,
        std::back_inserter(rotation_keys), [](aiQuatKey const& rot_key) {
          return RotationKey{
            static_cast<float>(rot_key.mTime), Convert(rot_key.mValue)
          };
        });

      std::vector<ScalingKey> scaling_keys;
      scaling_keys.reserve(ai_channel->mNumScalingKeys);
      std::ranges::transform(ai_channel->mScalingKeys, ai_channel->mScalingKeys + ai_channel->mNumScalingKeys,
        std::back_inserter(scaling_keys), [](aiVectorKey const& scale_key) {
          return ScalingKey{
            static_cast<float>(scale_key.mTime), Convert(scale_key.mValue)
          };
        });

      node_anims.emplace_back(std::move(position_keys), std::move(rotation_keys), std::move(scaling_keys),
        skeleton_node_name_to_idx.at(ai_channel->mNodeName.C_Str()));
    }

    animations.emplace_back(ai_anim->mName.C_Str(), static_cast<float>(ai_anim->mDuration),
      static_cast<float>(ai_anim->mTicksPerSecond), std::move(node_anims));
  }

  // Serialize

  // Element counts

  SerializeToBinary(std::size(mesh_data.positions), bytes);
  std::visit([&bytes]<typename T>(std::vector<T> const& indices) {
    SerializeToBinary(std::size(indices), bytes);
  }, mesh_data.indices);
  SerializeToBinary(std::ssize(mesh_data.material_slots), bytes);
  SerializeToBinary(std::size(mesh_data.sub_meshes), bytes);
  SerializeToBinary(animations.size(), bytes);
  SerializeToBinary(skeleton_nodes.size(), bytes);
  SerializeToBinary(bones.size(), bytes);
  SerializeToBinary(static_cast<std::int32_t>(std::holds_alternative<std::vector<std::uint32_t>>(mesh_data.indices)),
    bytes);

  // Vertex attributes and indices

  auto const pos_bytes{as_bytes(std::span{mesh_data.positions})};
  auto const norm_bytes{as_bytes(std::span{mesh_data.normals})};
  auto const uv_bytes{as_bytes(std::span{mesh_data.uvs})};
  auto const tan_bytes{as_bytes(std::span{mesh_data.tangents})};
  auto const idx_bytes{
    std::visit([]<typename T>(std::vector<T> const& indices) {
      return as_bytes(std::span{indices});
    }, mesh_data.indices)
  };
  auto const bone_weight_bytes{as_bytes(std::span{mesh_data.bone_weights})};
  auto const bone_idx_bytes{as_bytes(std::span{mesh_data.bone_indices})};

  bytes.reserve(
    std::size(bytes) + std::size(pos_bytes) + std::size(norm_bytes) + std::size(uv_bytes) + std::size(tan_bytes) +
    std::size(idx_bytes) + std::size(bone_weight_bytes) + std::size(bone_idx_bytes));

  std::ranges::copy(pos_bytes, std::back_inserter(bytes));
  std::ranges::copy(norm_bytes, std::back_inserter(bytes));
  std::ranges::copy(uv_bytes, std::back_inserter(bytes));
  std::ranges::copy(tan_bytes, std::back_inserter(bytes));
  std::ranges::copy(idx_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_weight_bytes, std::back_inserter(bytes));
  std::ranges::copy(bone_idx_bytes, std::back_inserter(bytes));

  // Material slots

  for (auto const& mtlSlot : mesh_data.material_slots) {
    SerializeToBinary(mtlSlot.name, bytes);
  }

  // Submeshes

  for (auto const& [baseVertex, firstIndex, indexCount, mtlSlotIdx, bounds] : mesh_data.sub_meshes) {
    SerializeToBinary(baseVertex, bytes);
    SerializeToBinary(firstIndex, bytes);
    SerializeToBinary(indexCount, bytes);
    SerializeToBinary(mtlSlotIdx, bytes);
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

  // Skeleton

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
