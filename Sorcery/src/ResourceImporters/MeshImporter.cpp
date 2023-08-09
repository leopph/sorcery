#include "MeshImporter.hpp"
#include "../Resources/Mesh.hpp"

#include <queue>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::MeshImporter>{"Mesh Importer"}
    .REFLECT_REGISTER_RESOURCE_IMPORTER_CTOR;
}


namespace sorcery {
namespace {
[[nodiscard]] auto Convert(aiVector3D const& aiVec) noexcept -> Vector3 {
  return Vector3{aiVec.x, aiVec.y, aiVec.z};
}


[[nodiscard]] auto Convert(aiMatrix4x4 const& aiMat) noexcept -> Matrix4 {
  return Matrix4{
    aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
    aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
    aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
    aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
  };
}
}


auto MeshImporter::GetSupportedFileExtensions(std::vector<std::string>& out) -> void {
  out.emplace_back(".fbx");
  out.emplace_back(".obj");
}


auto MeshImporter::Import(std::filesystem::path const& src) -> ObserverPtr<Resource> {
  Assimp::Importer importer;

  // Remove unnecessary scene elements
  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  // Remove point and line primitives
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

  auto const scene{importer.ReadFile(src.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_RemoveComponent | aiProcess_ConvertToLeftHanded)};

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw std::runtime_error{std::format("Failed to import model at {}: {}.", src.string(), importer.GetErrorString())};
  }

  // Convert all mesh data

  struct MeshInfo {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<unsigned> indices;
    unsigned mtlIdx{};
  };

  std::vector<MeshInfo> meshes;
  meshes.reserve(scene->mNumMeshes);

  for (unsigned i = 0; i < scene->mNumMeshes; i++) {
    // These meshes are always triangle-only, because
    // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines, 
    // aiProcess_Triangulate splits up primitives with more than 3 vertices
    // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
    // TODO Implement non-triangle rendering support

    aiMesh const* const mesh{scene->mMeshes[i]};
    auto& [vertices, normals, uvs, tangents, indices, mtlIdx]{meshes.emplace_back()};

    vertices.reserve(mesh->mNumVertices);
    normals.reserve(mesh->mNumVertices);
    uvs.reserve(mesh->mNumVertices);
    tangents.reserve(mesh->mNumVertices);

    for (unsigned j = 0; j < mesh->mNumVertices; j++) {
      vertices.emplace_back(Convert(mesh->mVertices[j]));
      normals.emplace_back(Normalized(Convert(mesh->mNormals[j])));
      uvs.emplace_back(mesh->HasTextureCoords(0)
                         ? Vector2{Convert(mesh->mTextureCoords[0][j])}
                         : Vector2{});
      tangents.emplace_back(Convert(mesh->mTangents[j]));
    }

    for (unsigned j = 0; j < mesh->mNumFaces; j++) {
      std::ranges::copy(std::span{mesh->mFaces[j].mIndices, mesh->mFaces[j].mNumIndices}, std::back_inserter(indices));
    }

    mtlIdx = mesh->mMaterialIndex;
  }

  // Flatten the hierarchy

  struct MeshReference {
    Matrix4 trafo;
    unsigned meshIdx;
  };

  std::vector<MeshReference> flatHierarchy;

  struct NodeProcessingInfo {
    Matrix4 accumParentTrafo;
    aiNode const* node;
  };

  std::queue<NodeProcessingInfo> queue;
  queue.emplace(Matrix4::Identity(), scene->mRootNode);

  while (!queue.empty()) {
    auto const& [accumParentTrafo, node] = queue.front();
    auto const trafo{Convert(node->mTransformation).Transpose() * accumParentTrafo};

    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
      flatHierarchy.emplace_back(trafo, node->mMeshes[i]);
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
      queue.emplace(trafo, node->mChildren[i]);
    }

    queue.pop();
  }

  // Sort and group meshes by material index

  struct SubmeshReference {
    std::vector<MeshReference> meshes;
  };

  std::map<unsigned, SubmeshReference> submeshes;

  for (auto const& [trafo, meshIdx] : flatHierarchy) {
    submeshes[meshes[meshIdx].mtlIdx].meshes.emplace_back(trafo, meshIdx);
  }

  // Transform the vertices and create submeshes around material indices

  Mesh::Data ret;

  for (auto const& [submeshMtlIdx, submeshRef] : submeshes) {
    int baseVertex{static_cast<int>(std::ssize(ret.positions))};
    int firstIndex{static_cast<int>(std::ssize(ret.indices))};
    int indexCount{0};

    // Only needed to offset the indices of the meshes forming the submesh
    int vertexCount{0};

    for (auto const& [trafo, meshIdx] : submeshRef.meshes) {
      auto const& [vertices, normals, uvs, tangents, indices, mtlIdx]{meshes[meshIdx]};

      ret.positions.reserve(ret.positions.size() + vertices.size());
      ret.normals.reserve(ret.normals.size() + normals.size());
      ret.uvs.reserve(ret.uvs.size() + uvs.size());
      ret.tangents.reserve(ret.tangents.size() + tangents.size());

      Matrix4 const trafoInvTransp{trafo.Inverse().Transpose()};

      for (int i = 0; i < std::ssize(vertices); i++) {
        ret.positions.emplace_back(Vector4{vertices[i], 1} * trafo);
        ret.normals.emplace_back(Vector4{normals[i], 0} * trafoInvTransp);
        ret.uvs.emplace_back(uvs[i]);
        ret.tangents.emplace_back(Vector4{tangents[i], 0} * trafoInvTransp);
      }

      // Submeshes use the baseVertex + firstIndex + indexCount technique but here we're composing a submesh from multiple assimp meshes so we manually have to offset each of their indices for the submesh
      std::ranges::transform(indices, std::back_inserter(ret.indices), [vertexCount](unsigned const idx) -> unsigned {
        return idx + vertexCount;
      });

      indexCount += static_cast<int>(std::ssize(indices));
      vertexCount += static_cast<int>(std::ssize(vertices));
    }

    ret.subMeshes.emplace_back(baseVertex, firstIndex, indexCount, scene->mMaterials[submeshMtlIdx]->GetName().C_Str());
  }

  return new Mesh{ret};
}


auto MeshImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  return rttr::type::get<Mesh>();
}
}
