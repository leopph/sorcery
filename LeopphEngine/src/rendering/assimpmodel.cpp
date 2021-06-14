#include "assimpmodel.h"
#include <utility>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

using leopph::impl::Shader;
using leopph::impl::Texture;
using leopph::impl::Vertex;
using leopph::impl::Mesh;

namespace leopph::impl
{
	AssimpModelImpl::AssimpModelImpl(std::filesystem::path path) :
		m_Path{ std::move(path) }
	{
		// read model data
		Assimp::Importer importer;
		const aiScene* scene{ importer.ReadFile(m_Path.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_MakeLeftHanded) };

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
			return;
		}

		m_Directory = m_Path.parent_path();

		// recursively process all nodes
		ProcessNode(scene->mRootNode, scene);
	}



	bool AssimpModelImpl::operator==(const AssimpModelImpl& other) const
	{
		return this->m_Directory == other.m_Directory;
	}



	void AssimpModelImpl::Draw(const Shader& shader) const
	{
		for (size_t i = 0; i < m_Meshes.size(); i++)
			m_Meshes[i].Draw(shader);
	}



	const std::filesystem::path& AssimpModelImpl::Path() const
	{
		return m_Path;
	}




	void AssimpModelImpl::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process all the meshes from current node
		for (unsigned i = 0; i < node->mNumMeshes; i++)
			m_Meshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));

		// recursively process child nodes
		for (unsigned i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}



	Mesh AssimpModelImpl::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		// tmp containers
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;
		std::vector<Texture> textures;

		// get vertex data
		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;
			vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

			if (mesh->mTextureCoords[0] != nullptr)
				vertex.textureCoordinates = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
			else
				vertex.textureCoordinates = { 0.0f, 0.0f };

			vertices.push_back(vertex);
		}

		// get index data
		for (unsigned i = 0; i < mesh->mNumFaces; i++)
			for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; j++)
				indices.push_back(mesh->mFaces[i].mIndices[j]);

		// get texture data
		aiMaterial* material{ scene->mMaterials[mesh->mMaterialIndex] };

		std::vector diffuseMaps{ LoadTexturesByType(material, aiTextureType_DIFFUSE, Texture::TextureType::DIFFUSE) };
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector specularMaps{ LoadTexturesByType(material, aiTextureType_SPECULAR, Texture::TextureType::SPECULAR) };
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		return Mesh(vertices, indices, textures);
	}



	std::vector<Texture> AssimpModelImpl::LoadTexturesByType(aiMaterial* material, aiTextureType assimpType, Texture::TextureType abstractType)
	{
		std::vector<Texture> textures;

		// iterate over all textures
		for (unsigned i = 0; i < material->GetTextureCount(assimpType); i++)
		{
			// read texture location
			aiString location;
			material->GetTexture(assimpType, i, &location);

			bool isAlreadyLoaded{ false };

			// if already loaded, reference that one
			for (const auto& texture : m_CachedTextures)
			{
				if (texture == m_Directory / location.C_Str())
				{
					textures.push_back(texture);
					isAlreadyLoaded = true;
					break;
				}
			}

			// if not loaded, load it
			if (!isAlreadyLoaded)
			{
				Texture texture{ m_Directory / location.C_Str(), abstractType };
				textures.push_back(texture);
				m_CachedTextures.push_back(texture);
			}
		}

		return textures;
	}
}