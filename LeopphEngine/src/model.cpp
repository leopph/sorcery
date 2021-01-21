#include "model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <glad/glad.h>
#include <stb_image.h>

namespace leopph
{
	Model::Model(const std::filesystem::path& path)
	{
		// read model data
		Assimp::Importer importer;
		const aiScene* scene{ importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_FlipUVs) };

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
			return;
		}

		directory = path.parent_path();

		// recursively process all nodes
		ProcessNode(scene->mRootNode, scene);
	}



	Model::Model(Model&& other) noexcept
	{
		this->meshes = std::move(other.meshes);
	}



	Model& Model::operator=(Model&& other) noexcept
	{
		this->meshes = std::move(other.meshes);
		this->directory = std::move(other.directory);
		this->m_LoadedTextures = std::move(other.m_LoadedTextures);

		other.directory.clear();

		return *this;
	}



	void Model::Draw(const Shader& shader) const
	{
		for (size_t i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}



	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// process all the meshes from current node
		for (unsigned i = 0; i < node->mNumMeshes; i++)
			meshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));

		// recursively process child nodes
		for (unsigned i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}



	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
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
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material{ scene->mMaterials[mesh->mMaterialIndex] };

			std::vector<Texture> diffuseMaps{ LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse") };
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			std::vector<Texture> specularMaps{ LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular") };
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
		}

		return Mesh(vertices, indices, textures);
	}



	std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* material, aiTextureType assimpType, Texture::TextureType abstractType)
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
			for (unsigned j = 0; j < m_LoadedTextures.size(); j++)
			{
				if (m_LoadedTextures[j] == location.C_Str())
				{
					textures.push_back(m_LoadedTextures[j]);
					isAlreadyLoaded = true;
					break;
				}
			}

			// if not loaded, load it
			if (!isAlreadyLoaded)
			{
				Texture texture{ directory / location.C_Str(), abstractType };
				textures.push_back(texture);
				m_LoadedTextures.push_back(texture);
			}
		}

		return textures;
	}
}