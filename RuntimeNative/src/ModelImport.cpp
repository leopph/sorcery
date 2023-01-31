#include "ModelImport.hpp"

#include "BinarySerializer.hpp"
#include "Compress.hpp"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


#include <fstream>
#include <map>
#include <queue>
#include <ranges>
#include <unordered_map>
#include <utility>


namespace leopph {
namespace {
[[nodiscard]] auto ConvertMatrix(aiMatrix4x4 const& aiMat) noexcept -> Matrix4 {
	return Matrix4{
		aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
		aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
		aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
		aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
	};
}

[[nodiscard]] auto ConvertVector(aiVector3D const& aiVec) noexcept -> Vector3 {
	return Vector3{ aiVec.x, aiVec.y, aiVec.z };
}
};


/*auto ImportModelAsset(std::filesystem::path const& srcFile) -> ModelImportData {
	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

	auto const scene{ importer.ReadFile(srcFile.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent) };

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error{ std::format("Failed to import model at {}: {}.", srcFile.string(), importer.GetErrorString()) };
	}

	ModelImportData ret;

	// Extract geometry data from the assimp structure, group it by the used materials, and calculate AABB for the meshes

	struct SubMeshData {
		std::vector<Vertex> vertices{};
		std::vector<u32> indices{};
	};

	// Maps a material index to a list of meshes
	std::map<std::size_t, std::vector<SubMeshData>> subMeshDataByMaterial;


	std::queue<std::pair<aiNode const*, Matrix4>> queue;
	queue.emplace(scene->mRootNode, ConvertMatrix(scene->mRootNode->mTransformation).Transpose() * Matrix4{ 1, 1, -1, 1 });

	while (!queue.empty()) {
		auto& [node, trafo] = queue.front();

		for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
			// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
			// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
			if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
				if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
					//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
					// TODO transform to triangle fans
				}

				auto& [vertices, indices]{ subMeshDataByMaterial[mesh->mMaterialIndex].emplace_back() };

				// Process the vertices
				for (unsigned j = 0; j < mesh->mNumVertices; j++) {
					vertices.emplace_back(
						Vector3{ Vector4{ ConvertVector(mesh->mVertices[j]), 1 } * trafo },
						Vector3{ Vector4{ ConvertVector(mesh->mNormals[j]), 0 } * trafo }.Normalize(),
						[mesh, j] {
							for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
								if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
									return Vector2{ ConvertVector(mesh->mTextureCoords[k][j]) };
								}
							}
							return Vector2{};
						}()
					);
				}

				// Process the indices
				for (unsigned j = 0; j < mesh->mNumFaces; j++) {
					for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++) {
						indices.push_back(mesh->mFaces[j].mIndices[k]);
					}
				}
			}
			else {
				std::string primitiveType;

				if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT) {
					primitiveType += " [points]";
				}

				if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) {
					primitiveType += " [lines]";
				}

				if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) {
					primitiveType += " [N>3 polygons]";
				}

				// TODO Implement non-triangle rendering support
				//Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType));
			}
		}

		for (std::size_t i = 0; i < node->mNumChildren; ++i) {
			queue.emplace(node->mChildren[i], ConvertMatrix(node->mChildren[i]->mTransformation).Transpose() * trafo);
		}

		queue.pop();
	}

	// Collapse meshes over materials and calculate their AABBs

	for(auto& subMeshes : subMeshDataByMaterial | std::views::values) {
		auto& mesh{ ret.meshes.emplace_back() };

		for (auto& [vertices, indices] : subMeshes) {
			for (auto& index : indices) {
				index += clamp_cast<u32>(mesh.vertices.size());
			}

			mesh.vertices.insert(std::end(mesh.vertices), std::begin(vertices), std::end(vertices));
			mesh.indices.insert(std::end(mesh.indices), std::begin(indices), std::end(indices));

			mesh.bounds = CalculateBounds(mesh.vertices);
		}
	}

	return ret;
}*/

auto ImportMeshResourceData(std::filesystem::path const& srcPath) -> MeshData {
	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

	auto const scene{ importer.ReadFile(srcPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent | aiProcess_FlipWindingOrder) };

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error{ std::format("Failed to import model at {}: {}.", srcPath.string(), importer.GetErrorString()) };
	}

	MeshData ret;

	struct TransformedNodeData {
		Matrix4 transform;
		aiNode const* node;
	};

	std::queue<TransformedNodeData> queue;
	queue.emplace(ConvertMatrix(scene->mRootNode->mTransformation).Transpose() * Matrix4{ 1, 1, -1, 1 }, scene->mRootNode);

	while (!queue.empty()) {
		auto const& [trafo, node] = queue.front();

		for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
			// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
			// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
			if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
				if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
					//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
					// TODO transform to triangle fans
				}

				auto const previousVertexCount{ clamp_cast<u32>(ret.positions.size()) };

				ret.positions.reserve(ret.positions.size() + mesh->mNumVertices);
				ret.normals.reserve(ret.normals.size() + mesh->mNumVertices);
				ret.uvs.reserve(ret.uvs.size() + mesh->mNumVertices);

				for (unsigned j = 0; j < mesh->mNumVertices; j++) {
					ret.positions.emplace_back(Vector4{ ConvertVector(mesh->mVertices[j]), 1 } * trafo);
					ret.normals.emplace_back(Normalized(Vector3{ Vector4{ ConvertVector(mesh->mNormals[j]), 0 } * trafo }));
					ret.uvs.emplace_back([mesh, j] {
						for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
							if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
								return Vector2{ ConvertVector(mesh->mTextureCoords[k][j]) };
							}
						}
						return Vector2{};
					}());
				}

				for (unsigned j = 0; j < mesh->mNumFaces; j++) {
					ret.indices.reserve(ret.indices.size() + mesh->mFaces[j].mNumIndices);

					for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++) {
						ret.indices.emplace_back(mesh->mFaces[j].mIndices[k] + previousVertexCount);
					}
				}
			}
			else {
				std::string primitiveType;

				if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT) {
					primitiveType += " [points]";
				}

				if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE) {
					primitiveType += " [lines]";
				}

				if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON) {
					primitiveType += " [N>3 polygons]";
				}

				// TODO Implement non-triangle rendering support
				//Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType));
			}
		}

		for (std::size_t i = 0; i < node->mNumChildren; ++i) {
			queue.emplace(ConvertMatrix(node->mChildren[i]->mTransformation).Transpose() * trafo, node->mChildren[i]);
		}

		queue.pop();
	}

	return ret;
}


auto CalculateBounds(std::span<Vertex const> const vertices) noexcept -> AABB {
	AABB bounds{};

	for (auto const& [position, normal, uv] : vertices) {
		bounds.min = Vector3{ std::min(bounds.min[0], position[0]), std::min(bounds.min[1], position[1]), std::min(bounds.min[2], position[2]) };
		bounds.max = Vector3{ std::max(bounds.max[0], position[0]), std::max(bounds.max[1], position[1]), std::max(bounds.max[2], position[2]) };
	}

	return bounds;
}
}
