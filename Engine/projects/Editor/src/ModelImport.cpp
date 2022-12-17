#include "ModelImport.hpp"


#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "YamlInclude.hpp"

#include <algorithm>
#include <cstring>
#include <format>
#include <optional>
#include <queue>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>


namespace leopph::editor {
	ModelAsset::ModelAsset(std::filesystem::path assetPath, StaticModelData const& modelData) :
		Asset{ std::move(assetPath) },
		mModelData{ modelData } {
	}

	auto ModelAsset::GetModelData() const -> StaticModelData const& {
		return mModelData;
	}

	auto ModelAsset::GenerateMetaFile() const -> std::string {
		YAML::Node node;
		node["assetType"] = static_cast<int>(AssetType::Model);
		YAML::Emitter emitter;
		emitter << node;
		return emitter.c_str();
	}


	auto ModelAssetLoader::CanLoad(std::filesystem::path const& assetPath) const -> bool {
		return mImporter.IsExtensionSupported(assetPath.extension().string());
	}


	auto ModelAssetLoader::Load(std::filesystem::path const& assetPath) -> std::shared_ptr<Asset> {
		// UTILITY FUNCTIONS

		auto const buildMatrix4 = [](aiMatrix4x4 const& aiMat) {
			return Matrix4
			{
				aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
				aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
				aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
				aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
			};
		};

		auto const buildVector3 = [](aiVector3D const& aiVec) {
			return Vector3{ aiVec.x, aiVec.y, aiVec.z };
		};

		auto const buildColor = [](aiColor3D const& aiCol) {
			return Color{ static_cast<u8>(aiCol.r * 255), static_cast<u8>(aiCol.g * 255), static_cast<u8>(aiCol.b * 255), 255 };
		};

		auto const loadTexture = [](aiString const& texPath, std::filesystem::path const& rootPath, aiScene const* const scene) {
			// Texture is embedded
			if (auto const* const embeddedTexture = scene->GetEmbeddedTexture(texPath.C_Str()); embeddedTexture) {
				// Texture is not compressed
				if (embeddedTexture->mHeight) {
					auto const numBytes = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
					auto imgData = std::make_unique_for_overwrite<u8[]>(numBytes);
					std::memcpy(imgData.get(), embeddedTexture->pcData, numBytes);

					return Image{ embeddedTexture->mWidth, embeddedTexture->mHeight, 4, std::move(imgData) };
				}

				// Texture is compressed
				//Logger::get_instance().error("Skipped loading embedded texture, becuase it was compressed. Compressed embedded textures are not yet supported."); TODO
				return Image{};
			}

			// Texture is in a separate file
			return Image{ rootPath / texPath.C_Str(), ImageOrientation::FlipVertical };
		};

		// Import with assimp

		mImporter.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

		auto const* scene = mImporter.ReadFile(assetPath.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent);


		// Return empty if import failed

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			//Logger::get_instance().error(std::format("Failed to import model at {}: {}.", assetPath.string(), importer.GetErrorString())); // TODO
			return {};
		}



		// Extract geometry data from the assimp structure, group it by the used materials, and calculate AABB for the meshes

		// Maps a material index to a list of meshes
		std::unordered_map<std::size_t, std::vector<StaticMeshData>> subMeshDataByMaterial;

		{
			std::queue<std::pair<aiNode const*, Matrix4>> queue;
			queue.emplace(scene->mRootNode, buildMatrix4(scene->mRootNode->mTransformation).transpose() * Matrix4 { 1, 1, -1, 1 });

			while (!queue.empty()) {
				auto& [node, trafo] = queue.front();

				for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
					// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
					// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
					if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
						if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
							//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string())); TODO
							// TODO transform to triangle fans
						}

						// Process the vertices

						std::vector<Vertex> vertices;

						for (unsigned j = 0; j < mesh->mNumVertices; j++) {
							Vertex vertex
							{
								.position = Vector3{Vector4{buildVector3(mesh->mVertices[j]), 1} *trafo},
								.normal = Vector3{Vector4{buildVector3(mesh->mNormals[j]), 0} *trafo}.normalize(),
								.uv = [mesh, j, &buildVector3] {
									for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
										if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
											return Vector2{buildVector3(mesh->mTextureCoords[k][j])};
										}
									}
									return Vector2{};
								}()
							};

							vertices.push_back(vertex);
						}

						// Process the indices

						std::vector<u32> indices;

						for (unsigned j = 0; j < mesh->mNumFaces; j++) {
							for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++) {
								indices.push_back(mesh->mFaces[j].mIndices[k]);
							}
						}

						// Calculate AABB

						AABB bounds{};

						for (auto const& [position, normal, uv] : vertices) {
							bounds.min = Vector3{ std::min(bounds.min[0], position[0]), std::min(bounds.min[1], position[1]), std::min(bounds.min[2], position[2]) };
							bounds.max = Vector3{ std::max(bounds.max[0], position[0]), std::max(bounds.max[1], position[1]), std::max(bounds.max[2], position[2]) };
						}

						subMeshDataByMaterial[mesh->mMaterialIndex].emplace_back(std::move(vertices), std::move(indices), bounds);
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

						//Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType)); TODO
					}
				}

				for (std::size_t i = 0; i < node->mNumChildren; ++i) {
					queue.emplace(node->mChildren[i], buildMatrix4(node->mChildren[i]->mTransformation).transpose() * trafo);
				}

				queue.pop();
			}
		}


		// Create the final data structure by collapsing meshes over materials, combining their AABBs, and loading their material data


		StaticModelData ret;

		{
			auto const& modelDir = assetPath.parent_path();

			// Caches loaded textures based on their file paths and indices in the array
			std::unordered_map<std::string, std::size_t> texturePathToIndex;

			for (auto& [materialIndex, mesh] : subMeshDataByMaterial) {
				{
					StaticMeshData meshAccum{};

					// Accumulate vertices and indices

					for (auto& [vertices, indices, aabb] : mesh) {
						for (auto& index : indices) {
							index += clamp_cast<u32>(meshAccum.vertices.size());
						}

						meshAccum.vertices.insert(std::end(meshAccum.vertices), std::begin(vertices), std::end(vertices));
						meshAccum.indices.insert(std::end(meshAccum.indices), std::begin(indices), std::end(indices));

						// Extend AABB

						meshAccum.boundingBox.min = Vector3{ std::min(meshAccum.boundingBox.min[0], aabb.min[0]), std::min(meshAccum.boundingBox.min[1], aabb.min[1]), std::min(meshAccum.boundingBox.min[2], aabb.min[2]) };
						meshAccum.boundingBox.max = Vector3{ std::max(meshAccum.boundingBox.max[0], aabb.max[0]), std::max(meshAccum.boundingBox.max[1], aabb.max[1]), std::max(meshAccum.boundingBox.max[2], aabb.max[2]) };
					}

					ret.meshes.emplace_back(std::move(meshAccum));
				}

				// Load the corresponding material data

				{
					MaterialData matData;

					{
						auto const* const aiMat = scene->mMaterials[materialIndex];

						if (aiColor3D diffClr; aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffClr) == aiReturn_SUCCESS) {
							matData.diffuseColor = buildColor(diffClr);
						}

						if (f32 opacity; aiMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS) {
							matData.diffuseColor.alpha = static_cast<u8>(opacity * 255);
						}

						if (aiColor3D specClr; aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specClr) == aiReturn_SUCCESS) {
							matData.specularColor = buildColor(specClr);
						}

						if (ai_real gloss; aiMat->Get(AI_MATKEY_SHININESS, gloss) == aiReturn_SUCCESS) {
							matData.gloss = gloss;
						}

						if (int twoSided; aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS) {
							matData.cullBackFace = !twoSided;
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texPath) == aiReturn_SUCCESS) {
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex)) {
								matData.diffuseMapIndex = it->second;
							}
							else {
								ret.textures.push_back(loadTexture(texPath, modelDir, scene));
								ret.textures.back().set_encoding(ColorEncoding::sRGB);
								texturePathToIndex[texPath.C_Str()] = ret.textures.size() - 1;
								matData.diffuseMapIndex = ret.textures.size() - 1;
							}
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texPath) == aiReturn_SUCCESS) {
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex)) {
								matData.specularMapIndex = it->second;
							}
							else {
								ret.textures.push_back(loadTexture(texPath, modelDir, scene));
								ret.textures.back().set_encoding(ColorEncoding::Linear);
								texturePathToIndex[texPath.C_Str()] = ret.textures.size() - 1;
								matData.specularMapIndex = ret.textures.size() - 1;
							}
						}
					}
					ret.materials.emplace_back(matData);
				}
			}
		}

		return std::make_shared<ModelAsset>(assetPath, ret);
	}
}