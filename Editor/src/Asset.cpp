#include "Asset.hpp"

#include <AABB.hpp>
#include <Mesh.hpp>
#include <Texture2D.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <queue>
#include <fstream>

#include "BinarySerializer.hpp"
#include <YamlInclude.hpp>
#include <Serialization.hpp>

#include "Material.hpp"

namespace leopph::editor {
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

[[nodiscard]] auto LoadMeshAsset(std::filesystem::path const& srcPath) -> std::shared_ptr<Mesh> {
	Assimp::Importer importer;

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

	auto const scene{ importer.ReadFile(srcPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent | aiProcess_FlipWindingOrder) };

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		throw std::runtime_error{ std::format("Failed to import model at {}: {}.", srcPath.string(), importer.GetErrorString()) };
	}

	Mesh::Data meshData;

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

				auto const previousVertexCount{ clamp_cast<u32>(meshData.positions.size()) };

				meshData.positions.reserve(meshData.positions.size() + mesh->mNumVertices);
				meshData.normals.reserve(meshData.normals.size() + mesh->mNumVertices);
				meshData.uvs.reserve(meshData.uvs.size() + mesh->mNumVertices);

				for (unsigned j = 0; j < mesh->mNumVertices; j++) {
					meshData.positions.emplace_back(Vector4{ ConvertVector(mesh->mVertices[j]), 1 } * trafo);
					meshData.normals.emplace_back(Normalized(Vector3{ Vector4{ ConvertVector(mesh->mNormals[j]), 0 } * trafo }));
					meshData.uvs.emplace_back([mesh, j] {
						for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
							if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
								return Vector2{ ConvertVector(mesh->mTextureCoords[k][j]) };
							}
						}
						return Vector2{};
					}());
				}

				for (unsigned j = 0; j < mesh->mNumFaces; j++) {
					meshData.indices.reserve(meshData.indices.size() + mesh->mFaces[j].mNumIndices);

					for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++) {
						meshData.indices.emplace_back(mesh->mFaces[j].mIndices[k] + previousVertexCount);
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

	return std::make_shared<Mesh>(std::move(meshData));
}

[[nodiscard]] auto LoadTexture2DAsset(std::filesystem::path const& filePath) -> std::shared_ptr<Texture2D> {
	return nullptr;
}

[[nodiscard]] auto LoadNativeAsset(std::filesystem::path const& filePath) -> std::shared_ptr<Object> {
	if (!filePath.has_extension()) {
		return nullptr;
	}

	if (filePath.extension() == ".mtl") {
		auto const node{ YAML::LoadFile(filePath.string()) };
		auto mtl{ std::make_shared<Material>() };
		mtl->SetAlbedoVector(node["albedo"].as<Vector3>(mtl->GetAlbedoVector()));
		mtl->SetMetallic(node["metallic"].as<float>(mtl->GetMetallic()));
		mtl->SetRoughness(node["roughness"].as<float>(mtl->GetRoughness()));
		mtl->SetAo(node["ao"].as<float>(mtl->GetAo()));
		return mtl;
	}

	return nullptr;
}
}

auto LoadAsset(std::filesystem::path const& filePath) -> std::shared_ptr<Object> {
	if (auto mesh{ LoadMeshAsset(filePath) }; mesh) {
		return mesh;
	}

	if (auto tex2D{ LoadTexture2DAsset(filePath) }; tex2D) {
		return tex2D;
	}

	return LoadNativeAsset(filePath);
}

auto GenerateAssetMetaFileContents(Object const& asset, EditorObjectFactoryManager const& factoryManager) -> std::string {
	YAML::Node node;
	node["guid"] = asset.GetGuid().ToString();
	node["type"] = static_cast<int>(asset.GetSerializationType());
	node["importPrecedence"] = factoryManager.GetFor(asset.GetSerializationType()).GetImporter().GetPrecedence();
	return Dump(node);
}

auto ReadAssetMetaFileContents(std::string const& contents) -> AssetMetaInfo {
	auto const node{ YAML::Load(contents) };
	if (!node["guid"] || !node["type"] || !node["importPrecedence"]) {
		throw std::runtime_error{ "Corrupt asset meta info." };
	}
	return AssetMetaInfo{ static_cast<Object::Type>(node["type"].as<int>()), Guid::Parse(node["guid"].as<std::string>()), node["importPrecedence"].as<int>() };
}

auto AssignAssetMetaContents(Object& asset, std::span<u8 const> const metaContentBytes) -> void {
	auto const node{ YAML::Load(std::string_view{ reinterpret_cast<char const*>(metaContentBytes.data()), metaContentBytes.size() }.data()) };
	asset.SetGuid(Guid::Parse(node["guid"].as<std::string>(asset.GetGuid().ToString())));
}
}
