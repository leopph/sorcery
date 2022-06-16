#include "Process.hpp"

#include "Image.hpp"
#include "Logger.hpp"
#include "TypeConversion.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <optional>
#include <queue>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>


namespace leopph::convert
{
	namespace
	{
		// Returns the index of the texture image corresponding to the target texture type in the passed material.
		// If the texture is not yet loaded, it gets loaded and stored in the vector along with its source path and index in the map.
		// Returns an empty optional if the texture could not be found or loaded.
		auto GetTexture(aiMaterial const* const mat, aiTextureType const texType, std::vector<Image>& textures, std::unordered_map<std::string, std::size_t>& idToInd, std::filesystem::path const& rootPath) -> std::optional<std::size_t>
		{
			if (aiString texPath; mat->GetTexture(texType, 0, &texPath) == aiReturn_SUCCESS)
			{
				if (idToInd.contains(texPath.C_Str()))
				{
					return idToInd[texPath.C_Str()];
				}

				textures.emplace_back(rootPath / texPath.C_Str(), true);
				return idToInd[texPath.C_Str()] = textures.size() - 1;
			}

			return {};
		}


		auto ProcessMaterials(std::span<aiMaterial* const> const aiMats, std::filesystem::path const& rootPath) -> std::pair<std::vector<Material>, std::vector<Image>>
		{
			std::unordered_map<std::string, std::size_t> idToInd;
			std::vector<Image> textures;
			std::vector<Material> materials;

			for (auto const* const aiMat : aiMats)
			{
				Material mat;

				if (float opacity; aiMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
				{
					mat.Opacity = opacity;
				}

				if (aiColor3D diffClr; aiMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffClr) == aiReturn_SUCCESS)
				{
					mat.DiffuseColor = Convert(diffClr);
				}

				if (aiColor3D specClr; aiMat->Get(AI_MATKEY_COLOR_SPECULAR, specClr) == aiReturn_SUCCESS)
				{
					mat.SpecularColor = Convert(specClr);
				}

				if (ai_real gloss; aiMat->Get(AI_MATKEY_SHININESS, gloss) == aiReturn_SUCCESS)
				{
					mat.Gloss = gloss;
				}

				if (int twoSided; aiMat->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
				{
					mat.TwoSided = !twoSided;
				}

				mat.DiffuseMap = GetTexture(aiMat, aiTextureType_DIFFUSE, textures, idToInd, rootPath);
				mat.SpecularMap = GetTexture(aiMat, aiTextureType_SPECULAR, textures, idToInd, rootPath);
				mat.OpacityMap = GetTexture(aiMat, aiTextureType_OPACITY, textures, idToInd, rootPath);

				// If the diffuse map has an alpha channel, and we couldn't parse an opacity map
				// We assume that the transparency comes from the diffuse alpha, so we steal it
				// And create an opacity map from that.
				if (!mat.OpacityMap.has_value() && mat.DiffuseMap.has_value() && textures[mat.DiffuseMap.value()].Channels() == 4)
				{
					auto& tex = textures[mat.DiffuseMap.value()];
					textures.push_back(tex.ExtractChannel(3));
					mat.OpacityMap = textures.size() - 1;
				}

				materials.push_back(mat);
			}

			return {std::move(materials), std::move(textures)};
		}


		auto ProcessVertices(aiMesh const* mesh, Matrix4 const& trafo) -> std::vector<Vertex>
		{
			std::vector<Vertex> vertices;

			for (unsigned i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex
				{
					.Position = Vector3{Vector4{Convert(mesh->mVertices[i]), 1} * trafo},
					.Normal = Vector3{Vector4{Convert(mesh->mNormals[i]), 0} * trafo},
					.TexCoord = [mesh, i]
					{
						for (std::size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
						{
							if (mesh->HasTextureCoords(static_cast<unsigned>(j)))
							{
								return Vector2{Convert(mesh->mTextureCoords[j][i])};
							}
						}
						return Vector2{};
					}()
				};

				vertices.push_back(vertex);
			}

			return vertices;
		}


		auto ProcessIndices(aiMesh const* mesh) -> std::vector<unsigned>
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


		auto LogPrimitiveError(aiMesh const* const mesh, std::filesystem::path const& path) -> void
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


	auto ProcessGenericModel(std::filesystem::path const& path) -> Object
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(path.string(),
		                                      aiProcess_JoinIdenticalVertices |
		                                      aiProcess_MakeLeftHanded |
		                                      aiProcess_Triangulate |
		                                      aiProcess_SortByPType |
		                                      aiProcess_GenUVCoords |
		                                      aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "Error parsing model from file at " << path << ": " << importer.GetErrorString() << '\n';
			return {};
		}

		Object object;

		auto [materials, textures] = ProcessMaterials(std::span{scene->mMaterials, scene->mNumMaterials}, path.parent_path());

		object.Materials = std::move(materials);
		object.Textures = std::move(textures);

		std::queue<std::pair<aiNode const*, Matrix4>> queue;
		queue.emplace(scene->mRootNode, Convert(scene->mRootNode->mTransformation));

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

					object.Meshes.emplace_back(ProcessVertices(mesh, trafo), ProcessIndices(mesh), mesh->mMaterialIndex);
				}
				else
				{
					LogPrimitiveError(mesh, path);
				}
			}

			for (std::size_t i = 0; i < node->mNumChildren; ++i)
			{
				queue.emplace(node->mChildren[i], Convert(node->mChildren[i]->mTransformation) * trafo);
			}

			queue.pop();
		}

		return object;
	}
}
