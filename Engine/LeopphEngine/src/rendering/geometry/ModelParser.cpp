#include "ModelParser.hpp"

#include "Logger.hpp"
#include "Texture.hpp"

#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <queue>
#include <utility>


namespace leopph::internal
{
	auto ModelParser::Parse(std::filesystem::path path) -> std::vector<Mesh>
	{
		m_Path = std::move(path);

		auto const* scene = m_Importer.ReadFile(m_Path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			Logger::Instance().Error(m_Importer.GetErrorString());
			return {};
		}

		m_Materials.clear();

		for (unsigned i = 0; i < scene->mNumMaterials; i++)
		{
			m_Materials.push_back(ProcessMaterial(scene->mMaterials[i]));
		}

		return ProcessNodes();
	}


	auto ModelParser::ProcessNodes() -> std::vector<Mesh>
	{
		std::queue<std::pair<aiNode*, Matrix4>> queue;
		std::vector<Mesh> ret;

		queue.emplace(m_Importer.GetScene()->mRootNode, ConvertTrafo(m_Importer.GetScene()->mRootNode->mTransformation) * Matrix4{1, 1, -1, 1});

		while (!queue.empty())
		{
			auto& [node, trafo] = queue.front();

			for (std::size_t i = 0; i < node->mNumMeshes; ++i)
			{
				if (auto const mesh = m_Importer.GetScene()->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes - aiPrimitiveType_TRIANGLE == 0)
				{
					ret.emplace_back(ProcessVertices(mesh, trafo), ProcessIndices(mesh), m_Materials[mesh->mMaterialIndex]);
				}
			}

			for (std::size_t i = 0; i < node->mNumChildren; ++i)
			{
				auto const childTrafo{ConvertTrafo(node->mChildren[i]->mTransformation)};
				queue.emplace(node->mChildren[i], trafo * childTrafo);
			}

			queue.pop();
		}

		return ret;
	}


	auto ModelParser::ProcessVertices(aiMesh const* mesh, Matrix4 const& trafo) -> std::vector<Vertex>
	{
		std::vector<Vertex> vertices;

		for (unsigned i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex;

			vertex.Position = Vector3{Vector4{mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1} * trafo};
			vertex.Normal = Vector3{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
			vertex.Normal = Vector3{Vector4{vertex.Normal, 0} * trafo};
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

		return vertices;
	}


	auto ModelParser::ProcessIndices(aiMesh const* mesh) -> std::vector<unsigned>
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


	auto ModelParser::ProcessMaterial(aiMaterial const* assimpMat) const -> std::shared_ptr<Material>
	{
		auto leopphMat{std::make_shared<Material>()};

		if (float opacity; assimpMat->Get(AI_MATKEY_OPACITY, opacity) == aiReturn_SUCCESS)
		{
			leopphMat->Opacity = opacity;
		}

		if (aiString texPath; assimpMat->GetTexture(aiTextureType_OPACITY, 0, &texPath) == aiReturn_SUCCESS)
		{
			if (auto const img{LoadTextureImage(texPath)}; !img.Empty())
			{
				leopphMat->OpacityMap = std::make_shared<Texture>(img);
			}
		}

		if (aiString texPath; assimpMat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS)
		{
			if (auto img{LoadTextureImage(texPath)}; !img.Empty())
			{
				// If the diffuse map has an alpha channel, and we couldn't parse an opacity map
				// We assume that the transparency comes from the diffuse alpha, so we steal it
				// And create an opacity map from that.
				if (img.Channels() == 4 && !leopphMat->OpacityMap)
				{
					auto alphaChan = img.ExtractChannel(3);
					leopphMat->OpacityMap = std::make_shared<Texture>(alphaChan);
				}

				leopphMat->DiffuseMap = std::make_shared<Texture>(img);
			}
		}

		if (aiString texPath; assimpMat->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == aiReturn_SUCCESS)
		{
			if (auto img{LoadTextureImage(texPath)}; !img.Empty())
			{
				leopphMat->SpecularMap = std::make_shared<Texture>(img);
			}
		}

		if (aiColor3D diffClr; assimpMat->Get(AI_MATKEY_COLOR_DIFFUSE, diffClr) == aiReturn_SUCCESS)
		{
			leopphMat->DiffuseColor = Color{Vector3{diffClr.r, diffClr.g, diffClr.b}};
		}

		if (aiColor3D specClr; assimpMat->Get(AI_MATKEY_COLOR_SPECULAR, specClr) == aiReturn_SUCCESS)
		{
			leopphMat->SpecularColor = Color{Vector3{specClr.r, specClr.g, specClr.b}};
		}

		if (ai_real gloss; assimpMat->Get(AI_MATKEY_SHININESS, gloss) == aiReturn_SUCCESS)
		{
			leopphMat->Gloss = gloss;
		}

		if (int twoSided; assimpMat->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
		{
			leopphMat->TwoSided = !twoSided;
		}

		return leopphMat;
	}


	auto ModelParser::LoadTextureImage(aiString const& texPath) const -> Image
	{
		if (auto const* texture = m_Importer.GetScene()->GetEmbeddedTexture(texPath.C_Str()))
		{
			Logger::Instance().Warning("Found embedded texture. Embedded textures are currently not supported.");
			return {};
		}

		return Image{m_Path.parent_path() / texPath.C_Str(), true};
	}


	auto ModelParser::ConvertTrafo(aiMatrix4x4 const& trafo) -> Matrix4
	{
		Matrix4 ret;
		for (unsigned i = 0; i < 4; ++i)
		{
			for (unsigned j = 0; j < 4; ++j)
			{
				ret[i][j] = trafo[j][i];
			}
		}
		return ret;
	}
}
