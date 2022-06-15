#include "LeopphverterCommon.hpp"
#include "LeopphverterImport.hpp"
#include "Logger.hpp"
#include "Matrix.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <queue>
#include <string>
#include <utility>


namespace leopph::convert
{
	namespace
	{
		auto Convert(aiMatrix4x4 const& aiMat) -> Matrix4
		{
			return Matrix4
			{
				aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
				aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
				aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
				aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
			}.Transpose();
		}


		auto Convert(aiVector3D const& aiVec) -> Vector3
		{
			return Vector3{aiVec.x, aiVec.y, aiVec.z};
		}


		auto Convert(aiColor3D const& aiCol) -> Color
		{
			return Color{static_cast<unsigned char>(aiCol.r * 255), static_cast<unsigned char>(aiCol.g * 255), static_cast<unsigned char>(aiCol.b * 255)};
		}


		auto ProcessMaterial(aiMaterial const* aiMat) -> Material
		{
			Material mat;

			if (float opacity; aiMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
			{
				mat.Opacity = opacity;
			}

			if (aiString texPath; aiMat->GetTexture(aiTextureType_OPACITY, 0, &texPath) == aiReturn_SUCCESS)
			{
				mat.OpacityMap = texPath.C_Str();
			}

			if (aiString texPath; aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS)
			{
				mat.DiffuseMap = texPath.C_Str();
			}

			if (aiString texPath; aiMat->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == aiReturn_SUCCESS)
			{
				mat.SpecularMap = texPath.C_Str();
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

			return mat;
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


	auto Import(std::filesystem::path const& path) -> Object
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
			std::cerr << "Error parsing model from file at " << path << ": " << importer.GetErrorString() << '\n';
			return {};
		}

		Object object;

		for (unsigned i = 0; i < scene->mNumMaterials; i++)
		{
			object.Materials.push_back(ProcessMaterial(scene->mMaterials[i]));
		}

		std::queue<std::pair<aiNode const*, Matrix4>> queue;

		queue.emplace(scene->mRootNode, Convert(scene->mRootNode->mTransformation) * Matrix4{1, 1, -1, 1});

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
				queue.emplace(node->mChildren[i], trafo * Convert(node->mChildren[i]->mTransformation));
			}

			queue.pop();
		}

		return object;
	}
}
