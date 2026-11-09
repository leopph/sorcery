#include "scene.hpp"

#include <ranges>
#include <span>
#include <stack>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <spdlog/spdlog.h>

#include "types.hpp"
#include "sorcery/char_encoding_helpers.hpp"


namespace sorcery::mage::model_import {
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
}
