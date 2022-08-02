#include "StaticMesh.hpp"

#include "GlCore.hpp"
#include "Renderer.hpp"
#include "../InternalContext.hpp"


namespace leopph
{
	StaticMesh::StaticMesh() :
		mVao{internal::GetRenderer()->request_vao()},
		mVbo{internal::GetRenderer()->request_buffer()},
		mIbo{internal::GetRenderer()->request_buffer()}
	{
		glNamedBufferData(mVbo, 1, nullptr, GL_STATIC_DRAW);
		glNamedBufferStorage(mIbo, 1, nullptr, GL_STATIC_DRAW);

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



	u32 StaticMesh::get_vao() const
	{
		return mVao;
	}



	StaticMesh::InstanceData* StaticMesh::get_instance_buffer(std::size_t const index) const
	{
		return mInstanceBufs[index].mapping;
	}



	u32 StaticMesh::get_instance_buffer_count() const
	{
		return NUM_INSTANCE_BUFFERS;
	}



	u32 StaticMesh::get_instance_buffer_size() const
	{
		return mInstanceBufSize;
	}



	void StaticMesh::resize_instance_buffers(u32 const numElements)
	{
		mInstanceBufSize = numElements * sizeof InstanceData;

		for (auto& [buffer, mapping] : mInstanceBufs)
		{
			glUnmapNamedBuffer(buffer);
			glNamedBufferData(buffer, mInstanceBufSize, nullptr, GL_DYNAMIC_DRAW);
			mapping = static_cast<InstanceData*>(glMapNamedBufferRange(buffer, 0, mInstanceBufSize, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));
		}
	}



	std::span<StaticMesh::SubMesh const> StaticMesh::get_sub_meshes() const
	{
		return mSubMeshes;
	}



	std::span<std::shared_ptr<Material const> const> StaticMesh::get_materials() const
	{
		return mMaterials;
	}



	void StaticMesh::set_materials(std::vector<std::shared_ptr<Material const>> materials)
	{
		mMaterials = std::move(materials);
	}



	void StaticMesh::set_vertices(std::span<Vertex const> const vertices)
	{
		glNamedBufferData(mVbo, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);
	}



	void StaticMesh::set_indices(std::span<u32 const> const indices)
	{
		glNamedBufferData(mIbo, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);
	}



	StaticMesh::~StaticMesh()
	{
		internal::GetRenderer()->release_vao(mVao);
		internal::GetRenderer()->release_buffer(mIbo);
		internal::GetRenderer()->release_buffer(mVbo);
	}
}
