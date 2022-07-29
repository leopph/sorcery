#include "StaticMesh.hpp"

#include "GlCore.hpp"


namespace leopph
{
	StaticMesh::StaticMesh()
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferData(mVbo, 1, nullptr, GL_STATIC_DRAW);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, 1, nullptr, GL_STATIC_DRAW);

		glCreateVertexArrays(1, &mVao);
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



	void StaticMesh::set_indices(std::span<u32 const> indices)
	{
		glNamedBufferData(mIbo, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);
	}



	StaticMesh::~StaticMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}
}
