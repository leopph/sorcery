#include "ModelParser.hpp"

#include "../../data/DataManager.hpp"
#include "../../util/Logger.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <memory>
#include <queue>
#include <utility>


namespace leopph::internal
{
	auto ModelParser::operator()(std::filesystem::path const& path) const -> std::vector<Mesh>
	{
		static Assimp::Importer importer;
		auto const scene{importer.ReadFile(path.string(), aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_SortByPType | aiProcess_GenUVCoords | aiProcess_GenNormals)};

		if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
		{
			Logger::Instance().Error(importer.GetErrorString());
			return {};
		}

		return ProcessNodes(scene, path);
	}


	auto ModelParser::ProcessNodes(aiScene const* const scene, std::filesystem::path const& path) -> std::vector<Mesh>
	{
		std::queue<std::pair<aiNode*, Matrix4>> queue;
		std::vector<Mesh> ret;

		queue.emplace(scene->mRootNode, ConvertTrafo(scene->mRootNode->mTransformation) * Matrix4{1, 1, -1, 1});

		while (!queue.empty())
		{
			auto& [node, trafo] = queue.front();

			for (std::size_t i = 0; i < node->mNumMeshes; ++i)
			{
				if (auto const mesh = scene->mMeshes[node->mMeshes[i]]; mesh->mPrimitiveTypes - aiPrimitiveType_TRIANGLE == 0)
				{
					ret.emplace_back(ProcessVertices(mesh, trafo), ProcessIndices(mesh), ProcessMaterial(scene, mesh, path));
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


	auto ModelParser::ProcessMaterial(aiScene const* scene, aiMesh const* mesh, std::filesystem::path const& path) -> std::shared_ptr<Material>
	{
		auto material{std::make_shared<Material>()};
		auto const assimpMaterial{scene->mMaterials[mesh->mMaterialIndex]};

		material->DiffuseMap = LoadTexture(assimpMaterial, aiTextureType_DIFFUSE, path);
		material->SpecularMap = LoadTexture(assimpMaterial, aiTextureType_SPECULAR, path);

		if (aiColor3D parsedDiffuseColor; assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, parsedDiffuseColor) == aiReturn_SUCCESS)
		{
			material->DiffuseColor = Color{Vector3{parsedDiffuseColor.r, parsedDiffuseColor.g, parsedDiffuseColor.b}};
		}

		if (aiColor3D parsedSpecularColor; assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, parsedSpecularColor) == aiReturn_SUCCESS)
		{
			material->SpecularColor = Color{Vector3{parsedSpecularColor.r, parsedSpecularColor.g, parsedSpecularColor.b}};
		}

		if (ai_real parsedGloss; assimpMaterial->Get(AI_MATKEY_SHININESS, parsedGloss) == aiReturn_SUCCESS)
		{
			material->Gloss = parsedGloss;
		}

		if (int twoSided; assimpMaterial->Get(AI_MATKEY_TWOSIDED, twoSided) == aiReturn_SUCCESS)
		{
			material->TwoSided = !twoSided;
		}

		return material;
	}


	auto ModelParser::LoadTexture(aiMaterial const* const material, aiTextureType const type, std::filesystem::path const& path) -> std::shared_ptr<Texture>
	{
		if (material->GetTextureCount(type) > 0)
		{
			aiString location;
			material->GetTexture(type, 0, &location);

			auto const texPath{path.parent_path() / location.C_Str()};

			if (auto p{DataManager::Instance().FindTexture(texPath)};
				p != nullptr)
			{
				return p;
			}

			return std::make_shared<Texture>(texPath);
		}
		return {};
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
