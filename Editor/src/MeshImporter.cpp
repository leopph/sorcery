#include "MeshImporter.hpp"

#include "Math.hpp"
#include "Serialization.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>
#include <queue>
#include <vector>

namespace leopph::editor {
class MeshImporter::Impl {
	Assimp::Importer mImporter;


	[[nodiscard]] static auto Convert(aiVector3D const& aiVec) noexcept -> Vector3 {
		return Vector3{ aiVec.x, aiVec.y, aiVec.z };
	}


	[[nodiscard]] static auto Convert(aiMatrix4x4 const& aiMat) noexcept -> Matrix4 {
		return Matrix4{
			aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
			aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
			aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
			aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
		};
	}

public:
	[[nodiscard]] auto GetSupportedExtensions() const -> std::string {
		aiString extensionList;
		mImporter.GetExtensionList(extensionList);
		std::string ret{ extensionList.C_Str() };
		std::erase(ret, '*');
		std::erase(ret, '.');
		return ret;
	}


	[[nodiscard]] auto Import(std::filesystem::path const& path) -> Mesh::Data {
		mImporter.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

		auto const scene{ mImporter.ReadFile(path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent | aiProcess_FlipWindingOrder | aiProcess_FlipUVs) };

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			throw std::runtime_error{ std::format("Failed to import model at {}: {}.", path.string(), mImporter.GetErrorString()) };
		}

		Mesh::Data ret;

		struct NodeProcessingInfo {
			Matrix4 accumParentTrafo;
			aiNode const* node;
		};

		std::queue<NodeProcessingInfo> queue;
		queue.emplace(Matrix4{ 1, 1, -1, 1 }, scene->mRootNode);

		while (!queue.empty()) {
			auto const& [accumParentTrafo, node] = queue.front();
			auto const trafo{ Convert(node->mTransformation).Transpose() * accumParentTrafo };
			auto const trafoInverseTranspose{ trafo.Inverse().Transpose() };

			for (unsigned i = 0; i < node->mNumMeshes; ++i) {
				// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
				// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
				if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
					if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
						//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
						// TODO transform to triangle fans
					}

					auto const prevVertCount{ ret.positions.size() };

					ret.positions.reserve(ret.positions.size() + mesh->mNumVertices);
					ret.normals.reserve(ret.normals.size() + mesh->mNumVertices);
					ret.uvs.reserve(ret.uvs.size() + mesh->mNumVertices);


					for (unsigned j = 0; j < mesh->mNumVertices; j++) {
						ret.positions.emplace_back(Vector4{ Convert(mesh->mVertices[j]), 1 } * trafo);
						ret.normals.emplace_back(Normalized(Vector3{ Vector4{ Convert(mesh->mNormals[j]), 0 } * trafoInverseTranspose }));
						ret.uvs.emplace_back([mesh, j] {
							for (int k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
								if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
									return Vector2{ Convert(mesh->mTextureCoords[k][j]) };
								}
							}
							return Vector2{};
						}());
					}

					auto const prevIdxCount{ ret.indices.size() };

					for (unsigned j = 0; j < mesh->mNumFaces; j++) {
						std::ranges::copy(std::span{ mesh->mFaces[j].mIndices, mesh->mFaces[j].mNumIndices }, std::back_inserter(ret.indices));
					}

					ret.subMeshes.emplace_back(clamp_cast<int>(prevVertCount), clamp_cast<int>(prevIdxCount), clamp_cast<int>(ret.indices.size() - prevIdxCount));
				}
				/*
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

					TODO Implement non-triangle rendering support
					Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType));
				}*/
			}

			for (unsigned i = 0; i < node->mNumChildren; ++i) {
				queue.emplace(trafo, node->mChildren[i]);
			}

			queue.pop();
		}

		return ret;
	}
};


MeshImporter::MeshImporter() : mImpl{ new Impl{} } {}


MeshImporter::~MeshImporter() {
	delete mImpl;
}


auto MeshImporter::GetSupportedExtensions() const -> std::string {
	return mImpl->GetSupportedExtensions();
}


auto MeshImporter::Import(InputImportInfo const& importInfo, std::filesystem::path const& cacheDir) -> Object* {
	auto const cachedDataPath{ cacheDir / importInfo.guid.ToString() };

	if (exists(cachedDataPath)) {
		std::ifstream in{ cachedDataPath, std::ios::binary };
		std::vector<unsigned char> fileData{ std::istreambuf_iterator{ in }, {} };
		std::span const bytes{ fileData };

		Mesh::Data meshData;
		auto const numVerts{ BinarySerializer<u64>::Deserialize(bytes.first<8>(), std::endian::little) };
		auto const numInds{ BinarySerializer<u64>::Deserialize(bytes.subspan<8, 8>(), std::endian::little) };
		auto const numSubMeshes{ BinarySerializer<u64>::Deserialize(bytes.subspan<16, 8>(), std::endian::little) };

		std::span const dataBytes{ bytes.subspan(3 * sizeof(u64)) };

		meshData.positions.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			meshData.positions.emplace_back(BinarySerializer<Vector3>::Deserialize(dataBytes.subspan(i * sizeof(Vector3)).first<sizeof(Vector3)>(), std::endian::little));
		}

		meshData.normals.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			meshData.normals.emplace_back(BinarySerializer<Vector3>::Deserialize(dataBytes.subspan(numVerts * sizeof(Vector3) + i * sizeof(Vector3)).first<sizeof(Vector3)>(), std::endian::little));
		}

		meshData.uvs.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numVerts; i++) {
			meshData.uvs.emplace_back(BinarySerializer<Vector2>::Deserialize(dataBytes.subspan(numVerts * 2 * sizeof(Vector3) + i * sizeof(Vector2)).first<sizeof(Vector2)>(), std::endian::little));
		}

		meshData.indices.reserve(numVerts);
		for (std::size_t i{ 0 }; i < numInds; i++) {
			meshData.indices.emplace_back(BinarySerializer<u32>::Deserialize(dataBytes.subspan(numVerts * (2 * sizeof(Vector3) + sizeof(Vector2)) + i * sizeof(u32)).first<sizeof(u32)>(), std::endian::little));
		}

		auto const subMeshBytes{ dataBytes.subspan(numVerts * (2 * sizeof(Vector3) + sizeof(Vector2)) + numInds * sizeof(u32)) };

		meshData.subMeshes.reserve(numSubMeshes);
		for (std::size_t i{ 0 }; i < numSubMeshes; i++) {
			auto const baseVertex{ BinarySerializer<int>::Deserialize(subMeshBytes.subspan(i * 3 * sizeof(int)).first<sizeof(int)>(), std::endian::little) };
			auto const firstIndex{ BinarySerializer<int>::Deserialize(subMeshBytes.subspan(i * 3 * sizeof(int) + sizeof(int)).first<sizeof(int)>(), std::endian::little) };
			auto const indexCount{ BinarySerializer<int>::Deserialize(subMeshBytes.subspan(i * 3 * sizeof(int) + 2 * sizeof(int)).first<sizeof(int)>(), std::endian::little) };
			meshData.subMeshes.emplace_back(baseVertex, firstIndex, indexCount);
		}

		return new Mesh{ std::move(meshData) };
	}

	auto meshData{ mImpl->Import(importInfo.src) };

	std::vector<u8> cachedData;

	BinarySerializer<u64>::Serialize(meshData.positions.size(), cachedData, std::endian::little);
	BinarySerializer<u64>::Serialize(meshData.indices.size(), cachedData, std::endian::little);
	BinarySerializer<u64>::Serialize(meshData.subMeshes.size(), cachedData, std::endian::little);

	for (auto const& pos : meshData.positions) {
		BinarySerializer<Vector3>::Serialize(pos, cachedData, std::endian::little);
	}

	for (auto const& norm : meshData.normals) {
		BinarySerializer<Vector3>::Serialize(norm, cachedData, std::endian::little);
	}

	for (auto const& uv : meshData.uvs) {
		BinarySerializer<Vector2>::Serialize(uv, cachedData, std::endian::little);
	}

	for (auto const ind : meshData.indices) {
		BinarySerializer<u32>::Serialize(ind, cachedData, std::endian::little);
	}

	for (auto const& [baseVertex, firstIndex, indexCount] : meshData.subMeshes) {
		BinarySerializer<int>::Serialize(baseVertex, cachedData, std::endian::little);
		BinarySerializer<int>::Serialize(firstIndex, cachedData, std::endian::little);
		BinarySerializer<int>::Serialize(indexCount, cachedData, std::endian::little);
	}

	std::ofstream out{ cachedDataPath, std::ios::out | std::ios::binary };
	std::ranges::copy(cachedData, std::ostreambuf_iterator{ out });

	return new Mesh{ std::move(meshData) };
}


auto MeshImporter::GetPrecedence() const noexcept -> int {
	return 0;
}
}
