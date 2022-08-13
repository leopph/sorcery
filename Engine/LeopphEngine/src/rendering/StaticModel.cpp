#include "StaticModel.hpp"

#include "Context.hpp"
#include "GlCore.hpp"
#include "Renderer.hpp"
#include "Util.hpp"


namespace leopph
{
	void StaticMaterial::bind_and_set_renderstate(u32 const index) const
	{
		if (mCullBackFace)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glBindBufferRange(GL_UNIFORM_BUFFER, index, mBuffer.get_internal_handle(), 0, clamp_cast<GLsizeiptr>(mBuffer.get_size()));
	}



	StaticMaterial::StaticMaterial(StaticMaterialData const& data, std::span<std::shared_ptr<Texture2D const> const> const textures) :
		mBuffer
		{
			// Buffer layout: 
			16 + // vec4 for diffuseColor
			16 + // vec4 for specular color + gloss
			8 +  // diffuseMap handle
			8 +  // specularMap handle
			4 +  // diffuseMap flag
			4 +  // specularMap flag
			4    // alpha threshold
		},
		mCullBackFace{data.cullBackFace}
	{
		auto* const bufferData = static_cast<u8*>(mBuffer.get_ptr());

		*reinterpret_cast<Vector4*>(bufferData) = Vector4{data.diffuseColor.red, data.diffuseColor.green, data.diffuseColor.blue, data.diffuseColor.alpha} / 255.f;
		*reinterpret_cast<Vector4*>(bufferData + 16) = Vector4{Vector3{data.specularColor.red, data.specularColor.green, data.specularColor.red} / 255.f, data.gloss};
		*reinterpret_cast<f32*>(bufferData + 56) = data.alphaThreshold;

		if (data.diffuseMapIndex)
		{
			mTextures.emplace_back(textures[*data.diffuseMapIndex]);
			*reinterpret_cast<u64*>(bufferData + 32) = mTextures.back()->get_handle();
			*reinterpret_cast<i32*>(bufferData + 48) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 32) = 0;
			*reinterpret_cast<i32*>(bufferData + 48) = false;
		}

		if (data.specularMapIndex)
		{
			mTextures.emplace_back(textures[*data.specularMapIndex]);
			*reinterpret_cast<u64*>(bufferData + 40) = mTextures.back()->get_handle();
			*reinterpret_cast<i32*>(bufferData + 52) = true;
		}
		else
		{
			*reinterpret_cast<u64*>(bufferData + 40) = 0;
			*reinterpret_cast<i32*>(bufferData + 52) = false;
		}
	}



	StaticMesh::StaticMesh(std::span<Vertex const> const vertices, std::span<u32 const> const indices) :
		mNumIndices{indices.size()}
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, vertices.size_bytes(), vertices.data(), 0);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, indices.size_bytes(), indices.data(), 0);

		glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, sizeof Vertex);
		glVertexArrayElementBuffer(mVao, mIbo);

		glVertexArrayAttribFormat(mVao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
		glVertexArrayAttribBinding(mVao, 0, 0);
		glEnableVertexArrayAttrib(mVao, 0);

		glVertexArrayAttribFormat(mVao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
		glVertexArrayAttribBinding(mVao, 1, 0);
		glEnableVertexArrayAttrib(mVao, 1);

		glVertexArrayAttribFormat(mVao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));
		glVertexArrayAttribBinding(mVao, 2, 0);
		glEnableVertexArrayAttrib(mVao, 2);
	}



	StaticMesh::~StaticMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}



	void StaticMesh::draw() const
	{
		glBindVertexArray(mVao);
		glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, nullptr);
	}



	StaticModel::StaticModel(StaticModelData const& data)
	{
		for (auto const& img : data.textures)
		{
			mTextures.emplace_back(std::make_shared<Texture2D>(img));
		}

		for (auto const& matData : data.materials)
		{
			mMaterials.emplace_back(std::make_unique<StaticMaterial>(matData, mTextures));
		}

		for (auto const& [vertices, indices, materialIndex] : data.meshes)
		{
			mMeshes.emplace_back(std::make_unique<StaticMesh>(vertices, indices));
		}
	}



	StaticModel::~StaticModel()
	{
		
	}
}
