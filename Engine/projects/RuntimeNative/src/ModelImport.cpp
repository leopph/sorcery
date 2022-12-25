#include "ModelImport.hpp"

#include "LeopphModelSerialize.hpp"

#include <queue>
#include <unordered_map>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Compress.hpp"

namespace leopph {
	auto LoadRawModelAsset(std::filesystem::path const& src) -> std::unique_ptr<Assimp::Importer> {
		auto importer{ std::make_unique<Assimp::Importer>() };

		importer->SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_ANIMATIONS | aiComponent_BONEWEIGHTS | aiComponent_CAMERAS | aiComponent_LIGHTS | aiComponent_COLORS);

		if (auto const scene{ importer->ReadFile(src.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveComponent) }; !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			throw std::runtime_error{ std::format("Failed to import model at {}: {}.", src.string(), importer->GetErrorString()) };
		}

		return importer;
	}


	auto ProcessRawModelAssetData(aiScene const& importedScene, std::filesystem::path const& src) -> ModelData {
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

		auto const loadTexture = [](aiString const& texPath, std::filesystem::path const& rootPath, aiScene const& scene) {
			// Texture is embedded
			if (auto const* const embeddedTexture = scene.GetEmbeddedTexture(texPath.C_Str()); embeddedTexture) {
				// Texture is not compressed
				if (embeddedTexture->mHeight) {
					auto const numBytes = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
					auto imgData = std::make_unique_for_overwrite<u8[]>(numBytes);
					std::memcpy(imgData.get(), embeddedTexture->pcData, numBytes);

					return Image{ embeddedTexture->mWidth, embeddedTexture->mHeight, 4, std::move(imgData) };
				}

				// Texture is compressed
				// TODO implement support for compressed embedded textures.
				//Logger::get_instance().error("Skipped loading embedded texture, becuase it was compressed. Compressed embedded textures are not yet supported.");
				return Image{};
			}

			// Texture is in a separate file
			return Image{ rootPath / texPath.C_Str(), ImageOrientation::FlipVertical };
		};

		// Extract geometry data from the assimp structure, group it by the used materials, and calculate AABB for the meshes

		// Maps a material index to a list of meshes
		std::unordered_map<std::size_t, std::vector<MeshData>> subMeshDataByMaterial;

		{
			std::queue<std::pair<aiNode const*, Matrix4>> queue;
			queue.emplace(importedScene.mRootNode, buildMatrix4(importedScene.mRootNode->mTransformation).transpose() * Matrix4{ 1, 1, -1, 1 });

			while (!queue.empty()) {
				auto& [node, trafo] = queue.front();

				for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
					// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
					// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
					if (auto const* const mesh = importedScene.mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) {
						if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag) {
							//Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
							// TODO transform to triangle fans
						}

						// Process the vertices

						std::vector<Vertex> vertices;

						for (unsigned j = 0; j < mesh->mNumVertices; j++) {
							Vertex vertex
							{
								.position = Vector3{ Vector4{ buildVector3(mesh->mVertices[j]), 1 } * trafo },
								.normal = Vector3{ Vector4{ buildVector3(mesh->mNormals[j]), 0 } * trafo }.normalize(),
								.uv = [mesh, j, &buildVector3] {
									for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++) {
										if (mesh->HasTextureCoords(static_cast<unsigned>(k))) {
											return Vector2{ buildVector3(mesh->mTextureCoords[k][j]) };
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

						// TODO Implement non-triangle rendering support
						//Logger::get_instance().debug(std::format("Ignoring non-triangle mesh in model file at {}. Primitives in the mesh are {}.", path.string(), primitiveType));
					}
				}

				for (std::size_t i = 0; i < node->mNumChildren; ++i) {
					queue.emplace(node->mChildren[i], buildMatrix4(node->mChildren[i]->mTransformation).transpose() * trafo);
				}

				queue.pop();
			}
		}


		// Create the final data structure by collapsing meshes over materials, combining their AABBs, and loading their material data


		ModelData ret;

		{
			auto const& modelDir = src.parent_path();

			// Caches loaded textures based on their file paths and indices in the array
			std::unordered_map<std::string, std::size_t> texturePathToIndex;

			for (auto& [materialIndex, mesh] : subMeshDataByMaterial) {
				{
					MeshData meshAccum{};

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
						auto const* const aiMat = importedScene.mMaterials[materialIndex];

						if (aiColor3D baseColor; aiMat->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS) {
							matData.albedo = buildColor(baseColor);
						}

						if (f32 metallic; aiMat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == aiReturn_SUCCESS) {
							matData.metallic = metallic;
						}

						if (f32 roughness; aiMat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn_SUCCESS) {
							matData.roughness = roughness;
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), texPath) == aiReturn_SUCCESS) {
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex)) {
								matData.albedoMapIndex = it->second;
							}
							else {
								ret.textures.push_back(loadTexture(texPath, modelDir, importedScene));
								ret.textures.back().set_encoding(ColorEncoding::sRGB);
								texturePathToIndex[texPath.C_Str()] = ret.textures.size() - 1;
								matData.albedoMapIndex = ret.textures.size() - 1;
							}
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_METALNESS, 0), texPath) == aiReturn_SUCCESS) {
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex)) {
								matData.metallicMapIndex = it->second;
							}
							else {
								ret.textures.push_back(loadTexture(texPath, modelDir, importedScene));
								ret.textures.back().set_encoding(ColorEncoding::sRGB);
								texturePathToIndex[texPath.C_Str()] = ret.textures.size() - 1;
								matData.metallicMapIndex = ret.textures.size() - 1;
							}
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE_ROUGHNESS, 0), texPath) == aiReturn_SUCCESS) {
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex)) {
								matData.roughnessMapIndex = it->second;
							}
							else {
								ret.textures.push_back(loadTexture(texPath, modelDir, importedScene));
								ret.textures.back().set_encoding(ColorEncoding::Linear);
								texturePathToIndex[texPath.C_Str()] = ret.textures.size() - 1;
								matData.roughnessMapIndex = ret.textures.size() - 1;
							}
						}
					}
					ret.materials.emplace_back(matData);
				}
			}
		}

		return ret;
	}


	auto GenerateLeopphAsset(ModelData const& modelData, std::vector<u8>& out, std::endian const endianness) -> std::vector<u8>& {
		if (endianness != std::endian::little && endianness != std::endian::big) {
			auto const msg = "Error while exporting. Mixed endian architectures are currently not supported.";
			//internal::Logger::Instance().Critical(msg);
			throw std::logic_error{ msg };
		}

		// signature bytes
		out.push_back('x');
		out.push_back('d');
		out.push_back('6');
		out.push_back('9');

		// the version number. its MSB is used to indicate if the file is little endian
		out.push_back(0x01 | (endianness == std::endian::big ? 0 : 0x80));

		std::vector<u8> toCompress;
		Serialize(modelData, out, endianness);

		switch (std::vector<u8> compressed; Compress(toCompress, compressed)) {
			case CompressionError::None: {
				Serialize(toCompress.size(), out, endianness);
				std::ranges::copy(compressed, std::back_inserter(out));
				break;
			}

			case CompressionError::Inconsistency: {
				throw std::runtime_error{ "Failed to compress asset file contents due to an inconsitency error." };
			}

			case CompressionError::Unknown: {
				throw std::runtime_error{ "Failed to compress asset file contents due to an unknown error." };
			}
		}

		return out;
	}
}
