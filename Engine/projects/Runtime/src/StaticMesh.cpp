#include "StaticMesh.hpp"

#include "GlContext.hpp"
#include "Util.hpp"


namespace leopph
{
	void StaticMesh::draw() const
	{
		glBindVertexArray(mVao);
		glDrawElementsInstanced(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, nullptr, mNumInstances);
	}



	void StaticMesh::set_instance_data(std::span<InstanceDataType const> const data)
	{
		if (data.size_bytes() > mInstanceBuf->get_size())
		{
			auto newBufSize = mInstanceBuf->get_size();

			while (newBufSize < data.size_bytes())
			{
				newBufSize *= 2;
			}

			mInstanceBuf = std::make_unique<PersistentMappedBuffer>(newBufSize);
		}

		std::memcpy(mInstanceBuf->get_ptr(), data.data(), data.size_bytes());
		mNumInstances = data.size();
	}



	AABB const& StaticMesh::get_bounding_box() const
	{
		return mBoundingBox;
	}



	void StaticMesh::register_entity(Node const* entity)
	{
		mReferringEntities.emplace(entity);
	}



	void StaticMesh::unregister_entity(Node const* entity)
	{
		mReferringEntities.erase(entity);
	}



	std::unordered_set<Node const*> StaticMesh::get_entities() const
	{
		return mReferringEntities;
	}



	StaticMesh::StaticMesh(StaticMeshData const& data) :
		mNumIndices{clamp_cast<u32>(data.indices.size())},
		mBoundingBox{data.boundingBox}
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, data.vertices.size() * sizeof Vertex, data.vertices.data(), 0);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, data.indices.size() * sizeof u32, data.indices.data(), 0);

		glCreateVertexArrays(1, &mVao);

		glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, sizeof Vertex);
		glVertexArrayVertexBuffer(mVao, 1, mInstanceBuf->get_internal_handle(), 0, sizeof InstanceDataType);
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

		glVertexArrayAttribFormat(mVao, 3, 4, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(mVao, 3, 1);
		glEnableVertexArrayAttrib(mVao, 3);

		glVertexArrayAttribFormat(mVao, 4, 4, GL_FLOAT, GL_FALSE, sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 4, 1);
		glEnableVertexArrayAttrib(mVao, 4);

		glVertexArrayAttribFormat(mVao, 5, 4, GL_FLOAT, GL_FALSE, 2 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 5, 1);
		glEnableVertexArrayAttrib(mVao, 5);

		glVertexArrayAttribFormat(mVao, 6, 4, GL_FLOAT, GL_FALSE, 3 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 6, 1);
		glEnableVertexArrayAttrib(mVao, 6);

		glVertexArrayAttribFormat(mVao, 7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 7, 1);
		glEnableVertexArrayAttrib(mVao, 7);

		glVertexArrayAttribFormat(mVao, 8, 4, GL_FLOAT, GL_FALSE, 5 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 8, 1);
		glEnableVertexArrayAttrib(mVao, 8);

		glVertexArrayAttribFormat(mVao, 9, 4, GL_FLOAT, GL_FALSE, 6 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 9, 1);
		glEnableVertexArrayAttrib(mVao, 9);

		glVertexArrayAttribFormat(mVao, 10, 4, GL_FLOAT, GL_FALSE, 7 * sizeof Vector4);
		glVertexArrayAttribBinding(mVao, 10, 1);
		glEnableVertexArrayAttrib(mVao, 10);

		glVertexArrayBindingDivisor(mVao, 1, 1);
	}



	StaticMesh::~StaticMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}
}
