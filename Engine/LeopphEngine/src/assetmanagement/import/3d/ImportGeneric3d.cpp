#include "ImportGeneric3d.hpp"

#include "Image.hpp"
#include "Logger.hpp"
#include "Matrix.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <cstring>
#include <format>
#include <optional>
#include <queue>
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
				internal::Logger::Instance().Error("Skipped loading embedded texture, becuase it was compressed. Compressed embedded textures are not yet supported.");
				return Image{};
			}

			// Texture is in a separate file
			return Image{rootPath / texPath.C_Str(), ImageOrientation::FlipVertical};
		}



		std::pair<std::vector<StaticMaterialData>, std::vector<Image>> process_materials(std::filesystem::path const& rootPath, aiScene const* const scene)
		{
			std::unordered_map<std::string, std::size_t> texturePathToIndex;
			std::vector<Image> textures;
			std::vector<StaticMaterialData> materials;

			for (unsigned i = 0; i < scene->mNumMaterials; i++)
			{
				auto const* const aiMat = scene->mMaterials[i];
				StaticMaterialData mat;

				if (aiColor3D diffClr; aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffClr) == aiReturn_SUCCESS)
				{
					mat.diffuseColor = convert(diffClr);
				}

				if (f32 opacity; aiMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
				{
					mat.diffuseColor.alpha = static_cast<u8>(opacity * 255);
				}

				if (aiColor3D specClr; aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specClr) == aiReturn_SUCCESS)
				{
					mat.specularColor = convert(specClr);
				}

				if (ai_real gloss; aiMat->Get(AI_MATKEY_SHININESS, gloss) == aiReturn_SUCCESS)
				{
					mat.gloss = gloss;
				}

				if (int twoSided; aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
				{
					mat.cullBackFace = !twoSided;
				}

				if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texPath) == aiReturn_SUCCESS)
				{
					if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex))
					{
						mat.diffuseMapIndex = it->second;
					}
					else
					{
						textures.push_back(load_texture(texPath, rootPath, scene));
						textures.back().set_encoding(ColorEncoding::sRGB);
						texturePathToIndex[texPath.C_Str()] = textures.size() - 1;
						mat.diffuseMapIndex = textures.size() - 1;
					}
				}

				if (aiString texPath; aiMat->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texPath) == aiReturn_SUCCESS)
				{
					if (auto const it = texturePathToIndex.find(texPath.C_Str()); it != std::end(texturePathToIndex))
					{
						mat.specularMapIndex = it->second;
					}
					else
					{
						textures.push_back(load_texture(texPath, rootPath, scene));
						textures.back().set_encoding(ColorEncoding::Linear);
						texturePathToIndex[texPath.C_Str()] = textures.size() - 1;
						mat.specularMapIndex = textures.size() - 1;
					}
				}

				materials.push_back(mat);
			}

			return {std::move(materials), std::move(textures)};
		}



		std::vector<Vertex> process_vertices(aiMesh const* mesh, Matrix4 const& trafo)
		{
			std::vector<Vertex> vertices;

			for (unsigned i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex
				{
					.position = Vector3{Vector4{convert(mesh->mVertices[i]), 1} * trafo},
					.normal = Vector3{Vector4{convert(mesh->mNormals[i]), 0} * trafo},
					.uv = [mesh, i]
					{
						for (std::size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
						{
							if (mesh->HasTextureCoords(static_cast<unsigned>(j)))
							{
								return Vector2{convert(mesh->mTextureCoords[j][i])};
							}
						}
						return Vector2{};
					}()
				};

				vertices.push_back(vertex);
			}

			return vertices;
		}



		std::vector<unsigned> process_indices(aiMesh const* mesh)
		{
			std::vector<unsigned> indices;

			for (unsigned i = 0; i < mesh->mNumFaces; i++)
			{
				for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; j++)
				{
					indices.push_back(mesh->mFaces[i].mIndices[j]);
				}
			}

			return indices;
		}



		void log_primitive_error(aiMesh const* const mesh, std::filesystem::path const& path)
		{
			std::string msg{"Ignoring mesh without triangles in model at path ["};
			msg += path.string();
			msg += "]. Primitives are";

			if (mesh->mPrimitiveTypes & aiPrimitiveType_POINT)
			{
				msg += " [points]";
			}

			if (mesh->mPrimitiveTypes & aiPrimitiveType_LINE)
			{
				msg += " [lines]";
			}

			if (mesh->mPrimitiveTypes & aiPrimitiveType_POLYGON)
			{
				msg += " [N>3 polygons]";
			}

			msg += ".";
			internal::Logger::Instance().Debug(msg);
		}
	}



	StaticModelData import_generic_static_meshes(std::filesystem::path const& path)
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(path.string(),
		                                      aiProcess_JoinIdenticalVertices |
		                                      aiProcess_Triangulate |
		                                      aiProcess_SortByPType |
		                                      aiProcess_GenUVCoords |
		                                      aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			internal::Logger::Instance().Error(std::format("Error parsing model file at [{}]: {}.", path.string(), importer.GetErrorString()));
			return {};
		}

		StaticModelData data;

		auto [materials, textures] = process_materials(path.parent_path(), scene);

		data.materials = std::move(materials);
		data.textures = std::move(textures);

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
						internal::Logger::Instance().Debug("Found NGON encoded mesh in model at path [" + path.string() + "].");
						// TODO currently ignoring NGON property
					}

					data.meshes.emplace_back(process_vertices(mesh, trafo), process_indices(mesh), mesh->mMaterialIndex);
				}
				else
				{
					log_primitive_error(mesh, path);
				}
			}

			for (std::size_t i = 0; i < node->mNumChildren; ++i)
			{
				queue.emplace(node->mChildren[i], convert(node->mChildren[i]->mTransformation) * trafo);
			}

			queue.pop();
		}

		return data;
	}
}
