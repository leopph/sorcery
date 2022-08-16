#include "ImportGeneric3d.hpp"

#include "Image.hpp"
#include "Logger.hpp"
#include "Matrix.hpp"
#include "Util.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cstring>
#include <format>
#include <optional>
#include <queue>
#include <ranges>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>


namespace leopph
{
	namespace
	{
		// Transposes the matrix
		Matrix4 convert(aiMatrix4x4 const& aiMat)
		{
			return Matrix4
			{
				aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
				aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
				aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
				aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
			};
		}



		Vector3 convert(aiVector3D const& aiVec)
		{
			return Vector3{aiVec.x, aiVec.y, aiVec.z};
		}



		Color convert(aiColor3D const& aiCol)
		{
			return Color{static_cast<u8>(aiCol.r * 255), static_cast<u8>(aiCol.g * 255), static_cast<u8>(aiCol.b * 255)};
		}



		Image load_texture(aiString const& texPath, std::filesystem::path const& rootPath, aiScene const* const scene)
		{
			// Texture is embedded
			if (auto const* const embeddedTexture = scene->GetEmbeddedTexture(texPath.C_Str()); embeddedTexture)
			{
				// Texture is not compressed
				if (embeddedTexture->mHeight)
				{
					auto const numBytes = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
					auto imgData = std::make_unique_for_overwrite<u8[]>(numBytes);
					std::memcpy(imgData.get(), embeddedTexture->pcData, numBytes);

					return Image{embeddedTexture->mWidth, embeddedTexture->mHeight, 4, std::move(imgData)};
				}

				// Texture is compressed
				Logger::get_instance().error("Skipped loading embedded texture, becuase it was compressed. Compressed embedded textures are not yet supported.");
				return Image{};
			}

			// Texture is in a separate file
			return Image{rootPath / texPath.C_Str(), ImageOrientation::FlipVertical};
		}
	}



	StaticModelData import_generic_static_model(std::filesystem::path const& path)
	{
		// Import with assimp

		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals);



		// Return empty if import failed

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Logger::get_instance().error(std::format("Failed to import model at {}: {}.", path.string(), importer.GetErrorString()));
			return {};
		}



		// Extract geometry data from the assimp structure and group it by the used materials


		struct SubMeshData
		{
			std::vector<Vertex> vertices;
			std::vector<u32> indices;
		};


		// Maps a material index to a list of submeshes
		std::unordered_map<std::size_t, std::vector<SubMeshData>> subMeshDataByMaterial;

		{
			std::queue<std::pair<aiNode const*, Matrix4>> queue;
			queue.emplace(scene->mRootNode, convert(scene->mRootNode->mTransformation) * Matrix4{1, 1, -1, 1});

			while (!queue.empty())
			{
				auto& [node, trafo] = queue.front();

				for (std::size_t i = 0; i < node->mNumMeshes; ++i)
				{
					// aiProcess_SortByPType will separate mixed-primitive meshes, so every mesh in theory should be clean and only contain one kind of primitive.
					// Testing for one type only is therefore safe, but triangle meshes have to be checked for NGON encoding too.
					if (auto const* const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
					{
						if (mesh->mPrimitiveTypes & aiPrimitiveType_NGONEncodingFlag)
						{
							Logger::get_instance().debug(std::format("Found NGON encoded submesh in model file at {}.", path.string()));
							// TODO transform to triangle fans
						}

						std::vector<Vertex> vertices;

						for (unsigned j = 0; j < mesh->mNumVertices; j++)
						{
							Vertex vertex
							{
								.position = Vector3{Vector4{convert(mesh->mVertices[j]), 1} * trafo},
								.normal = Vector3{Vector4{convert(mesh->mNormals[j]), 0} * trafo},
								.uv = [mesh, j]
								{
									for (std::size_t k = 0; k < AI_MAX_NUMBER_OF_TEXTURECOORDS; k++)
									{
										if (mesh->HasTextureCoords(static_cast<unsigned>(k)))
										{
											return Vector2{convert(mesh->mTextureCoords[k][j])};
										}
									}
									return Vector2{};
								}()
							};

							vertices.push_back(vertex);
						}

						std::vector<u32> indices;

						for (unsigned j = 0; j < mesh->mNumFaces; j++)
						{
							for (unsigned k = 0; k < mesh->mFaces[j].mNumIndices; k++)
							{
								indices.push_back(mesh->mFaces[j].mIndices[k]);
							}
						}

						subMeshDataByMaterial[mesh->mMaterialIndex].emplace_back(std::move(vertices), std::move(indices));
					}
					else
					{
						std::string primitiveType;

						if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT)
						{
							primitiveType += " [points]";
						}

						if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE)
						{
							primitiveType += " [lines]";
						}

						if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON)
						{
							primitiveType += " [N>3 polygons]";
						}

						Logger::get_instance().debug(std::format("Ignoring non-triangle submesh in model file at {}. Primitives in the submesh are {}.", path.string(), primitiveType));
					}
				}

				for (std::size_t i = 0; i < node->mNumChildren; ++i)
				{
					queue.emplace(node->mChildren[i], convert(node->mChildren[i]->mTransformation) * trafo);
				}

				queue.pop();
			}
		}


		// Collape the list of submeshes that use the same material and load the material data

		// Holds the same number of submeshes and materials.
		// Element at the same index correspond to each other.
		struct
		{
			std::vector<SubMeshData> subMeshes;
			std::vector<StaticMaterialData> materials;
		} subMeshesWithMaterials;

		std::vector<Image> textures;

		{
			auto const& modelDir = path.parent_path();
			// Caches loaded textures based on their file paths and indices in the array
			std::unordered_map<std::string, std::size_t> texturePathToIndex;

			for (auto& [materialIndex, subMeshes] : subMeshDataByMaterial)
			{
				// Accumulate the submeshes mapped to the material into one single entry

				{
					SubMeshData subMeshAccum;

					for (auto& [vertices, indices] : subMeshes)
					{
						for (auto& index : indices)
						{
							index += clamp_cast<u32>(subMeshAccum.vertices.size());
						}

						subMeshAccum.vertices.insert(std::end(subMeshAccum.vertices), std::begin(vertices), std::end(vertices));
						subMeshAccum.indices.insert(std::end(subMeshAccum.indices), std::begin(indices), std::end(indices));
					}
					subMeshesWithMaterials.subMeshes.emplace_back(std::move(subMeshAccum));
				}

				// Load the corresponding material data

				{
					StaticMaterialData matData;

					{
						auto const* const aiMat = scene->mMaterials[materialIndex];

						if (aiColor3D diffClr; aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffClr) == aiReturn_SUCCESS)
						{
							matData.diffuseColor = convert(diffClr);
						}

						if (f32 opacity; aiMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
						{
							matData.diffuseColor.alpha = static_cast<u8>(opacity * 255);
						}

						if (aiColor3D specClr; aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specClr) == aiReturn_SUCCESS)
						{
							matData.specularColor = convert(specClr);
						}

						if (ai_real gloss; aiMat->Get(AI_MATKEY_SHININESS, gloss) == aiReturn_SUCCESS)
						{
							matData.gloss = gloss;
						}

						if (int twoSided; aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
						{
							matData.cullBackFace = !twoSided;
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texPath) == aiReturn_SUCCESS)
						{
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex))
							{
								matData.diffuseMapIndex = it->second;
							}
							else
							{
								textures.push_back(load_texture(texPath, modelDir, scene));
								textures.back().set_encoding(ColorEncoding::sRGB);
								texturePathToIndex[texPath.C_Str()] = textures.size() - 1;
								matData.diffuseMapIndex = textures.size() - 1;
							}
						}

						if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texPath) == aiReturn_SUCCESS)
						{
							if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex))
							{
								matData.specularMapIndex = it->second;
							}
							else
							{
								textures.push_back(load_texture(texPath, modelDir, scene));
								textures.back().set_encoding(ColorEncoding::Linear);
								texturePathToIndex[texPath.C_Str()] = textures.size() - 1;
								matData.specularMapIndex = textures.size() - 1;
							}
						}
					}
					subMeshesWithMaterials.materials.emplace_back(matData);
				}
			}
		}


		// Create the final data structure by collapsing the submeshes into one big buffer while keeping their order in line with the material list

		StaticModelData ret{};
		ret.textures = std::move(textures);
		ret.materials = std::move(subMeshesWithMaterials.materials);

		for (auto const& [vertices, indices] : subMeshesWithMaterials.subMeshes)
		{
			AABB bounds{};

			for (auto const& [position, normal, uv] : vertices)
			{
				// Update submesh bounding box

				bounds.min = Vector3{std::min(bounds.min[0], position[0]), std::min(bounds.min[1], position[1]), std::min(bounds.min[2], position[2])};
				bounds.max = Vector3{std::max(bounds.max[0], position[0]), std::max(bounds.max[1], position[1]), std::max(bounds.max[2], position[2])};

				// Update whole mesh bounding box

				ret.mesh.boundingBox.min = Vector3{std::min(ret.mesh.boundingBox.min[0], position[0]), std::min(ret.mesh.boundingBox.min[1], position[1]), std::min(ret.mesh.boundingBox.min[2], position[2])};
				ret.mesh.boundingBox.max = Vector3{std::max(ret.mesh.boundingBox.max[0], position[0]), std::max(ret.mesh.boundingBox.max[1], position[1]), std::max(ret.mesh.boundingBox.max[2], position[2])};
			}

			SubMeshDescriptor const descriptor
			{
				.indexOffset = clamp_cast<i32>(ret.mesh.indices.size()), 
				.indexCount = clamp_cast<i32>(indices.size()), 
				.baseVertex = clamp_cast<i32>(ret.mesh.vertices.size()), 
				.boundingBox = bounds
			};

			ret.mesh.vertices.insert(std::end(ret.mesh.vertices), std::begin(vertices), std::end(vertices));
			ret.mesh.indices.insert(std::end(ret.mesh.indices), std::begin(indices), std::end(indices));
			ret.mesh.subMeshes.emplace_back(descriptor);
		}

		return ret;
	}
}
