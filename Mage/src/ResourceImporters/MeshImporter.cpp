#include "MeshImporter.hpp"
#include "../Resources/Mesh.hpp"
#include "../FileIo.hpp"
#include "Serialization.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <algorithm>
#include <limits>
#include <queue>
#include <ranges>

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


auto MeshImporter::GetSupportedFileExtensions(std::pmr::vector<std::string>& out) -> void {
  std::string extensions;
  Assimp::Importer const importer;
  importer.GetExtensionList(extensions);

  // Assimp extension list format is "*.3ds;*.obj;*.dae"
  for (auto const ext : std::views::split(extensions, std::string_view{";"})) {
    out.emplace_back(std::string_view{std::begin(ext), std::end(ext)}.substr(1));
  }
}


auto MeshImporter::Import(std::filesystem::path const& src, std::vector<std::byte>& bytes, ExternalResourceCategory& categ) -> bool {
  std::vector<unsigned char> meshBytes;

  if (!ReadFileBinary(src, meshBytes)) {
    return false;
  }

  Assimp::Importer importer;

  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);
  importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f);

  auto const scene{importer.ReadFileFromMemory(meshBytes.data(), meshBytes.size(), aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_TransformUVCoords | aiProcess_RemoveComponent)};

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    throw std::runtime_error{std::format("Failed to import model at {}: {}.", src.string(), importer.GetErrorString())};
  }

  Mesh::Data meshData;

  // Collect all info from aiMaterials

  meshData.materialSlots.resize(scene->mNumMaterials);

  for (unsigned i{0}; i < scene->mNumMaterials; i++) {
    meshData.materialSlots[i].name = scene->mMaterials[i]->GetName().C_Str();
  }

  // Collect all info from aiMeshes

  struct MeshProcessingData {
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    std::vector<Vector3> tangents;
    std::vector<unsigned> indices;
    unsigned mtlIdx{};
  };

  // Collected mesh data not yet transformed by node matrices
  std::vector<MeshProcessingData> meshesUntransformed;
  meshesUntransformed.reserve(scene->mNumMeshes);

  for (unsigned i = 0; i < scene->mNumMeshes; i++) {
    // These meshes are always triangle-only, because
    // AI_CONFIG_PP_SBP_REMOVE is set to remove points and lines, 
    // aiProcess_Triangulate splits up primitives with more than 3 vertices
    // aiProcess_SortByPType splits up meshes with more than 1 primitive type into homogeneous ones
    // TODO Implement non-triangle rendering support

    aiMesh const* const mesh{scene->mMeshes[i]};
    auto& [vertices, normals, uvs, tangents, indices, mtlIdx]{meshesUntransformed.emplace_back()};

    if (!mesh->HasPositions() || !mesh->HasNormals() || !mesh->HasTextureCoords(0) || !mesh->HasTangentsAndBitangents()) {
      // TODO log or something
      continue;
    }

    vertices.reserve(mesh->mNumVertices);
    normals.reserve(mesh->mNumVertices);
    uvs.reserve(mesh->mNumVertices);
    tangents.reserve(mesh->mNumVertices);

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
  }

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
    auto& [vertices, normals, uvs, tangents, indices, mtlIdx]{meshesTransformed.emplace_back(meshesUntransformed[meshIdx])};

    Matrix4 const trafoInvTransp{trafo.Inverse().Transpose()};

    for (int i = 0; i < std::ssize(vertices); i++) {
      vertices[i] = Vector3{Vector4{vertices[i], 1} * trafo};
      normals[i] = Vector3{Vector4{normals[i], 0} * trafoInvTransp};
      tangents[i] = Vector3{Vector4{tangents[i], 0} * trafoInvTransp};
    }
  }

  // Store geometry data and create submeshes

  meshData.subMeshes.reserve(std::size(meshesTransformed));
  meshData.indices.emplace<std::vector<std::uint32_t>>();

  for (auto const& [vertices, normals, uvs, tangents, indices, mtlIdx] : meshesTransformed) {
    meshData.subMeshes.emplace_back(static_cast<int>(std::ssize(meshData.positions)), std::visit([]<typename T>(std::vector<T> const& indices) { return static_cast<int>(std::ssize(indices)); }, meshData.indices), static_cast<int>(std::ssize(indices)), static_cast<int>(mtlIdx), AABB{});

    std::ranges::copy(vertices, std::back_inserter(meshData.positions));
    std::ranges::copy(normals, std::back_inserter(meshData.normals));
    std::ranges::copy(uvs, std::back_inserter(meshData.uvs));
    std::ranges::copy(tangents, std::back_inserter(meshData.tangents));
    std::ranges::copy(indices, std::back_inserter(std::get<std::vector<std::uint32_t>>(meshData.indices)));
  }

  // Transform indices to 16-bit if possible

  if (auto const& indices32{std::get<std::vector<std::uint32_t>>(meshData.indices)}; std::ranges::all_of(indices32, [](std::uint32_t const idx) {
    return idx <= std::numeric_limits<std::uint16_t>::max();
  })) {
    std::vector<std::uint16_t> indices16;
    indices16.reserve(std::size(indices32));

    std::ranges::transform(indices32, std::back_inserter(indices16), [](std::uint32_t const idx) {
      return static_cast<std::uint16_t>(idx);
    });
  }

  SerializeToBinary(std::size(meshData.positions), bytes);
  std::visit([&bytes]<typename T>(std::vector<T> const& indices) {
    SerializeToBinary(std::size(indices), bytes);
  }, meshData.indices);
  SerializeToBinary(std::ssize(meshData.materialSlots), bytes);
  SerializeToBinary(std::size(meshData.subMeshes), bytes);
  SerializeToBinary(static_cast<std::int32_t>(std::holds_alternative<std::vector<std::uint32_t>>(meshData.indices)), bytes);

  auto const posBytes{as_bytes(std::span{meshData.positions})};
  auto const normBytes{as_bytes(std::span{meshData.normals})};
  auto const uvBytes{as_bytes(std::span{meshData.uvs})};
  auto const tanBytes{as_bytes(std::span{meshData.tangents})};
  auto const idxBytes{
    std::visit([]<typename T>(std::vector<T> const& indices) {
      return as_bytes(std::span{indices});
    }, meshData.indices)
  };

  bytes.reserve(std::size(bytes) + std::size(posBytes) + std::size(normBytes) + std::size(uvBytes) + std::size(tanBytes) + std::size(idxBytes));

  std::ranges::copy(posBytes, std::back_inserter(bytes));
  std::ranges::copy(normBytes, std::back_inserter(bytes));
  std::ranges::copy(uvBytes, std::back_inserter(bytes));
  std::ranges::copy(tanBytes, std::back_inserter(bytes));
  std::ranges::copy(idxBytes, std::back_inserter(bytes));

  for (auto const& mtlSlot : meshData.materialSlots) {
    SerializeToBinary(mtlSlot.name, bytes);
  }

  for (auto const& [baseVertex, firstIndex, indexCount, mtlSlotIdx, bounds] : meshData.subMeshes) {
    SerializeToBinary(baseVertex, bytes);
    SerializeToBinary(firstIndex, bytes);
    SerializeToBinary(indexCount, bytes);
    SerializeToBinary(mtlSlotIdx, bytes);
  }

  categ = ExternalResourceCategory::Mesh;
  return true;
}


auto MeshImporter::GetImportedType(std::filesystem::path const& resPathAbs) noexcept -> rttr::type {
  return rttr::type::get<Mesh>();
}
}
