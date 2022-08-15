#include "StaticMesh.hpp"

#include "Context.hpp"
#include "GlCore.hpp"
#include "Renderer.hpp"
#include "Util.hpp"


namespace leopph
{
	void StaticMesh::draw_sub_mesh(std::size_t const index) const
	{
		glBindVertexArray(mVao);
		glDrawElementsBaseVertex(GL_TRIANGLES, mSubMeshes[index].indexCount, GL_UNSIGNED_INT, reinterpret_cast<void const*>(mSubMeshes[index].indexOffset), mSubMeshes[index].baseVertex);
	}



	std::size_t StaticMesh::get_sub_mesh_count() const
	{
		return mSubMeshes.size();
	}



	void StaticMesh::set_active_instance_buffer(std::size_t index)
	{ }



	AABB const& StaticMesh::get_bounding_box() const
	{
		return mBoundingBox;
	}



	void StaticMesh::register_entity(Entity const* entity)
	{
		mReferringEntities.emplace(entity);
	}



	void StaticMesh::unregister_entity(Entity const* entity)
	{
		mReferringEntities.erase(entity);
	}



	std::unordered_set<Entity const*> StaticMesh::get_entities() const
	{
		return mReferringEntities;
	}



	StaticMesh::StaticMesh(StaticMeshData const& data) :
		mInstanceBuf{PersistentMappedBuffer{mNumInstances}}
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, data.vertices.size() * sizeof Vertex, data.vertices.data(), 0);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, data.indices.size() * sizeof u32, data.indices.data(), 0);

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

		internal::get_renderer()->register_static_mesh(weak_from_this());
	}



	StaticMesh::~StaticMesh()
	{
		internal::get_renderer()->unregister_static_mesh(weak_from_this());

		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}
}
