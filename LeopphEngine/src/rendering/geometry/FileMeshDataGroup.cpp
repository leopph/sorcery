#include "FileMeshDataGroup.hpp"

#include "../../data/DataManager.hpp"
#include "../../util/logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <memory>
#include <queue>
#include <utility>


namespace leopph::internal
{
	FileMeshDataGroup::FileMeshDataGroup(std::filesystem::path path) :
		MeshDataGroup{path.generic_string()},
		m_Path{std::move(path)}
	{
		Assimp::Importer importer;
		const auto scene{importer.ReadFile(m_Path.string(), aiProcess_Triangulate | aiProcess_GenNormals)};

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			Logger::Instance().Error(importer.GetErrorString());
			return;
		}

		Data() = ProcessNodes(scene);
	}

	auto FileMeshDataGroup::Path() const -> const std::filesystem::path&
	{
		return m_Path;
	}

	auto FileMeshDataGroup::ProcessNodes(const aiScene* const scene) const -> std::vector<internal::MeshData>
	{
		struct NodeAndTransform
		{
			aiNode* Node{};
			Matrix3 Transform{};
		};

		std::queue<NodeAndTransform> nodes;

		Matrix3 rootTrafo;
		for (unsigned i = 0; i < 3; ++i)
		{
			for (unsigned j = 0; j < 3; ++j)
			{
				rootTrafo[i][j] = scene->mRootNode->mTransformation[j][i];
			}
		}
		rootTrafo *= Matrix3{1, 1, -1};
		nodes.emplace(scene->mRootNode, rootTrafo);

		std::vector<internal::MeshData> ret;

		while (!nodes.empty())
		{
			auto& [node, trafo] = nodes.front();

			for (std::size_t i = 0; i < node->mNumMeshes; ++i)
			{
				ret.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene, trafo));
			}

			for (std::size_t i = 0; i < node->mNumChildren; ++i)
			{
				Matrix3 childTrafo;
				for (unsigned j = 0; j < 3; ++j)
				{
					for (unsigned k = 0; k < 3; ++k)
					{
						childTrafo[j][k] = node->mChildren[i]->mTransformation[k][j];
					}
				}
				nodes.emplace(node->mChildren[i], trafo * childTrafo);
			}

			nodes.pop();
		}

		return ret;
	}

	auto FileMeshDataGroup::ProcessMesh(const aiMesh* const mesh, const aiScene* const scene, const Matrix3& trafo) const -> internal::MeshData
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;

		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.Position = Vector3{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z} * trafo;
			vertex.Normal = Vector3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z} * trafo;

			// ONLY 1 UV LAYER SUPPORTED
			vertex.TexCoord = Vector2{0.0f, 0.0f};
			for (std::size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
			{
				if (mesh->HasTextureCoords(static_cast<unsigned>(j)))
				{
					vertex.TexCoord = Vector2{mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y};
					break;
				}
			}

			vertices.push_back(vertex);
		}

		for (unsigned i = 0; i < mesh->mNumFaces; i++)
		{
			for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			{
				indices.push_back(mesh->mFaces[i].mIndices[j]);
			}
		}

		const auto assimpMaterial{scene->mMaterials[mesh->mMaterialIndex]};
		const auto material{std::make_shared<Material>()};

		material->AmbientMap = LoadTexture(assimpMaterial, aiTextureType_AMBIENT);
		material->DiffuseMap = LoadTexture(assimpMaterial, aiTextureType_DIFFUSE);
		material->SpecularMap = LoadTexture(assimpMaterial, aiTextureType_SPECULAR);

		if (ai_real parsedShininess;
			assimpMaterial->Get(AI_MATKEY_SHININESS, parsedShininess) == aiReturn_SUCCESS)
		{
			material->Gloss = static_cast<decltype(Material::Gloss)>(parsedShininess);
		}

		if (aiColor3D parsedDiffuseColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, parsedDiffuseColor) == aiReturn_SUCCESS)
		{
			material->DiffuseColor = {
				static_cast<unsigned char>(parsedDiffuseColor.r * 255),
				static_cast<unsigned char>(parsedDiffuseColor.g * 255), static_cast<unsigned char>(parsedDiffuseColor.b * 255)
			};
		}

		if (aiColor3D parsedSpecularColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, parsedSpecularColor) == aiReturn_SUCCESS)
		{
			material->SpecularColor = {
				static_cast<unsigned char>(parsedSpecularColor.r * 255),
				static_cast<unsigned char>(parsedSpecularColor.g * 255), static_cast<unsigned char>(parsedSpecularColor.b * 255)
			};
		}

		if (aiColor3D parsedAmbientColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, parsedAmbientColor) == aiReturn_SUCCESS)
		{
			material->AmbientColor = {
				static_cast<unsigned char>(parsedAmbientColor.r * 255),
				static_cast<unsigned char>(parsedAmbientColor.g * 255), static_cast<unsigned char>(parsedAmbientColor.b * 255)
			};
		}

		if (int twoSided;
			assimpMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
		{
			material->TwoSided = !twoSided;
		}

		return internal::MeshData(vertices, indices, material);
	}

	auto FileMeshDataGroup::LoadTexture(const aiMaterial* const material, const aiTextureType type) const -> std::shared_ptr<Texture>
	{
		if (material->GetTextureCount(type) > 0)
		{
			aiString location;
			material->GetTexture(type, 0, &location);

			Logger::Instance().Debug("Loading texture on path [" + (m_Path.parent_path() / location.C_Str()).string() + "].");

			const auto texPath{m_Path.parent_path() / location.C_Str()};

			if (auto p{DataManager::Instance().FindTexture(texPath)};
				p != nullptr)
			{
				return p;
			}

			return std::make_shared<Texture>(texPath);
		}
		return {};
	}
}
