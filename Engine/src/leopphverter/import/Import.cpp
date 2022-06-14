#include "Import.hpp"

#include "../Common.hpp"
#include "../../../LeopphEngine/src/math/Matrix.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <iostream>
#include <queue>
#include <utility>


namespace leopph::convert
{
	namespace
	{
		auto Convert(aiMatrix4x4 const& aiMat) -> Matrix4
		{
			Matrix4 ret;
			for (auto i = 0; i < 4; i++)
			{
				for (auto j = 0; j < 4; j++)
				{
					ret[i][j] = aiMat[i][j];
				}
			}
			return ret;
		}


		auto Convert(aiVector3D const& aiVec) -> Vector3
		{
			return Vector3{aiVec.x, aiVec.y, aiVec.z};
		}


		auto ProcessMaterial(aiMaterial const* aiMat) -> Material
		{
			return Material{}; // TODO
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
	}


	auto Import(std::filesystem::path const& path) -> Object
	{
		Assimp::Importer importer;
		auto const* scene = importer.ReadFile(path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals);

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
				if (auto const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes - aiPrimitiveType_TRIANGLE == 0)
				{
					object.Meshes.emplace_back(ProcessVertices(mesh, trafo), ProcessIndices(mesh), mesh->mMaterialIndex);
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
