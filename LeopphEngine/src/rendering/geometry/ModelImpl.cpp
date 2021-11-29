#include "ModelImpl.hpp"

#include "../Material.hpp"
#include "../../util/logger.h"

#include <assimp/Importer.hpp>
#include <assimp/matrix4x4.h>
#include <assimp/postprocess.h>
#include <glad/gl.h>

#include <algorithm>
#include <type_traits>
#include <queue>
#include <utility>



namespace leopph::impl
{
	ModelImpl::ModelImpl(std::filesystem::path path) :
		Path{std::move(path)}, m_Directory{Path.parent_path()}, m_InstanceBufferSize{1}, m_InstanceBuffer{}, m_CastsShadow{false}
	{
		Assimp::Importer importer;
		const auto scene{importer.ReadFile(Path.string(), aiProcess_Triangulate | aiProcess_GenNormals)};

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			Logger::Instance().Error(importer.GetErrorString());
			return;
		}

		glCreateBuffers(1, &m_InstanceBuffer);
		glNamedBufferData(m_InstanceBuffer, 2 * sizeof(Matrix4), nullptr, GL_STATIC_DRAW);

		ProcessNodes(scene);


	}


	void ModelImpl::DrawShaded(leopph::impl::ShaderProgram& shader, const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices, const std::size_t nextFreeTextureUnit) const
	{
		AdjustInstanceBuffer(instanceMatrices);

		for (const auto& mesh : m_Meshes)
		{
			mesh.DrawShaded(shader, nextFreeTextureUnit, instanceMatrices.size());
		}
	}


	void ModelImpl::DrawDepth(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const
	{
		AdjustInstanceBuffer(instanceMatrices);

		for (const auto& mesh : m_Meshes)
		{
			mesh.DrawDepth(instanceMatrices.size());
		}
	}


	bool ModelImpl::CastsShadow() const
	{
		return m_CastsShadow;
	}


	void ModelImpl::CastsShadow(const bool value)
	{
		m_CastsShadow = value;
	}


	void ModelImpl::AdjustInstanceBuffer(const std::vector<std::pair<Matrix4, Matrix4>>& instanceMatrices) const
	{
		if (instanceMatrices.size() > m_InstanceBufferSize)
		{
			m_InstanceBufferSize *= 2;
			glNamedBufferData(m_InstanceBuffer, m_InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_STATIC_DRAW);
		}
		else if (instanceMatrices.size() * 2 < m_InstanceBufferSize)
		{
			m_InstanceBufferSize = std::max(m_InstanceBufferSize / 2, 1ull);
			glNamedBufferData(m_InstanceBuffer, m_InstanceBufferSize * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data(), GL_STATIC_DRAW);
		}
		else
		{
			glNamedBufferSubData(m_InstanceBuffer, 0, instanceMatrices.size() * sizeof(std::remove_reference_t<decltype(instanceMatrices)>::value_type), instanceMatrices.data());
		}
	}


	void ModelImpl::ProcessNodes(const aiScene* scene)
	{
		struct NodeWithTrafo
		{
			aiNode* node;
			Matrix3 trafo;
		};

		std::queue<NodeWithTrafo> nodes;

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

		while (!nodes.empty())
		{
			auto& [node, trafo] = nodes.front();

			for (std::size_t i = 0; i < node->mNumMeshes; ++i)
			{
				m_Meshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene, trafo));
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
	}


	Mesh ModelImpl::ProcessMesh(const aiMesh* mesh, const aiScene* scene, const Matrix3& trafo) const
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;

		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position = Vector3{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z} * trafo;
			vertex.normal = Vector3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z} * trafo;

			// ONLY 1 UV LAYER SUPPORTED
			vertex.textureCoordinates = Vector2{0.0f, 0.0f};
			for (std::size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
			{
				if (mesh->HasTextureCoords(static_cast<unsigned>(j)))
				{
					vertex.textureCoordinates = Vector2{mesh->mTextureCoords[j][i].x, mesh->mTextureCoords[j][i].y};
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
		Material material;

		material.AmbientMap = LoadTexturesByType(assimpMaterial, aiTextureType_AMBIENT);
		material.DiffuseMap = LoadTexturesByType(assimpMaterial, aiTextureType_DIFFUSE);
		material.SpecularMap = LoadTexturesByType(assimpMaterial, aiTextureType_SPECULAR);

		if (ai_real parsedShininess;
			assimpMaterial->Get(AI_MATKEY_SHININESS, parsedShininess) == aiReturn_SUCCESS)
		{
			material.Shininess = static_cast<decltype(Material::Shininess)>(parsedShininess);
		}

		if (aiColor3D parsedDiffuseColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, parsedDiffuseColor) == aiReturn_SUCCESS)
		{
			material.DiffuseColor = {
				static_cast<unsigned char>(parsedDiffuseColor.r * 255),
				static_cast<unsigned char>(parsedDiffuseColor.g * 255), static_cast<unsigned char>(parsedDiffuseColor.b * 255)
			};
		}

		if (aiColor3D parsedSpecularColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, parsedSpecularColor) == aiReturn_SUCCESS)
		{
			material.SpecularColor = {
				static_cast<unsigned char>(parsedSpecularColor.r * 255),
				static_cast<unsigned char>(parsedSpecularColor.g * 255), static_cast<unsigned char>(parsedSpecularColor.b * 255)
			};
		}
		
		if (aiColor3D parsedAmbientColor;
			assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, parsedAmbientColor) == aiReturn_SUCCESS)
		{
			material.AmbientColor = {
				static_cast<unsigned char>(parsedAmbientColor.r * 255),
				static_cast<unsigned char>(parsedAmbientColor.g * 255), static_cast<unsigned char>(parsedAmbientColor.b * 255)
			};
		}

		if (int twoSided;
			assimpMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
		{
			material.TwoSided = !twoSided;
		}

		return Mesh(vertices, indices, std::move(material), m_InstanceBuffer);
	}


	std::optional<Texture> ModelImpl::LoadTexturesByType(const aiMaterial* material, const aiTextureType assimpType) const
	{
		if (material->GetTextureCount(assimpType) > 0)
		{
			aiString location;
			material->GetTexture(assimpType, 0, &location);

			Logger::Instance().Debug("Loading texture on path [" + (m_Directory / location.C_Str()).string() + "].");
			return std::make_optional<Texture>(m_Directory / location.C_Str());
		}

		Logger::Instance().Debug("Mesh contains no texture of the requested type.");
		return {};
	}
}
