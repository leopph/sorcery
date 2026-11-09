#include "scene.hpp"

#include <ranges>
#include <span>
#include <stack>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include "hash.hpp"
#include "types.hpp"
#include "sorcery/char_encoding_helpers.hpp"


namespace sorcery::mage::model_import {
namespace {
struct NodeListHash {
  auto operator()(std::span<NodeIndex const> const nodes) const noexcept -> std::size_t {
    auto h = Mix64(nodes.size());

    for (NodeIndex const n : nodes) {
      h = Mix64(h ^ n);
    }

    return h;
  }
};


struct NodeListEq {
  auto operator()(std::span<NodeIndex const> const lhs, std::span<NodeIndex const> const rhs) const noexcept -> bool {
    return std::ranges::equal(lhs, rhs);
  }
};
}


auto Convert(aiVector3D const& ai_vec) noexcept -> Vector3 {
  return Vector3{ai_vec.x, ai_vec.y, ai_vec.z};
}


auto Convert(aiQuaternion const& ai_quat) noexcept -> Quaternion {
  return Quaternion{ai_quat.w, ai_quat.x, ai_quat.y, ai_quat.z};
}


auto Convert(aiMatrix4x4 const& ai_mat) noexcept -> Matrix4 {
  return Matrix4{
    ai_mat.a1, ai_mat.a2, ai_mat.a3, ai_mat.a4,
    ai_mat.b1, ai_mat.b2, ai_mat.b3, ai_mat.b4,
    ai_mat.c1, ai_mat.c2, ai_mat.c3, ai_mat.c4,
    ai_mat.d1, ai_mat.d2, ai_mat.d3, ai_mat.d4
  };
}


auto LoadScene(std::filesystem::path const& src) -> std::unique_ptr<Assimp::Importer> {
  if (spdlog::get_level() == spdlog::level::trace) {
    Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE, aiDefaultLogStream_STDOUT | aiDefaultLogStream_DEBUGGER);
  }

  auto importer = std::make_unique<Assimp::Importer>();

  // We don't need these scene objects
  importer->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  // We don't want to bother with non-triangle primitives
  importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  // Smoothing angle for smooth normal generation
  importer->SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);
  // By default, assimp’s FBX importer keeps pivot transforms as extra nodes named …$AssimpFbx$_Translation, …_Rotation and so on.
  // The hierarchy would show them as clutter. This flag folds them into a single transform per node.
  importer->SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

  auto const scene{
    importer->ReadFile(std::string{ToUntypedSv(src.u8string())},
      aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_TransformUVCoords |
      aiProcess_RemoveComponent)
  };

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    spdlog::error("Failed to import model at {}: {}.", ToUntypedSv(src.u8string()), importer->GetErrorString());
    return nullptr;
  }

  return std::move(importer);
}


auto FlattenNodeHierarchy(aiScene const& scene) -> SceneNodeList {
  SceneNodeList list;

  std::stack<SceneNode> stack;
  stack.emplace(MakeObserver(scene.mRootNode), std::nullopt);

  while (!stack.empty()) {
    auto const node = stack.top();
    stack.pop();

    list.emplace_back(node);

    for (auto const* const child : std::views::reverse(std::span{node.ptr->mChildren, node.ptr->mNumChildren})) {
      // Already placed at least one node in the list, so minus one is safe
      stack.emplace(MakeObserver(child), list.size() - 1);
    }
  }

  return list;
}


auto CollectNodesByMesh(std::span<SceneNode const> const nodes,
                        std::size_t const mesh_count) -> NodesByMesh {
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


auto CollectMeshesByGroup(NodesByMesh const& nodes_by_mesh,
                          std::span<std::optional<SourceMesh> const> const meshes) -> MeshesByGroup {
  std::unordered_map<std::span<NodeIndex const>, MeshGroupIndex, NodeListHash, NodeListEq> groups_by_node_list;
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
auto CollectGroupsByNode(MeshesByGroup const& meshes_by_group, NodesByMesh const& nodes_by_mesh,
                         std::size_t const node_count) -> GroupsByNode {
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
}
