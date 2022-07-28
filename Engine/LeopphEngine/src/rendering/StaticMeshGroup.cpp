#include "StaticMeshGroup.hpp"

#include "GlCore.hpp"


namespace leopph
{
	StaticMeshGroup::StaticMesh::StaticMesh(std::span<Vertex const> const vertices, std::span<u32> const indices) :
		mNumIndices{indices.size()}
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, vertices.size_bytes(), vertices.data(), 0);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, indices.size_bytes(), indices.data(), 0);

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



	void StaticMeshGroup::StaticMesh::draw() const
	{
		glBindVertexArray(mVao);
		glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_INT, nullptr);
	}



	StaticMeshGroup::StaticMesh::~StaticMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}



	StaticMeshGroup::StaticMeshGroup(std::vector<std::unique_ptr<StaticMesh>> meshes, std::vector<std::shared_ptr<Material>> materials) :
		mMeshes{std::move(meshes)},
		mMaterials{std::move(materials)}
	{ }



	std::span<std::unique_ptr<StaticMeshGroup::StaticMesh> const> StaticMeshGroup::get_meshes() const
	{
		return mMeshes;
	}



	std::span<std::shared_ptr<Material> const> StaticMeshGroup::get_materials() const
	{
		return mMaterials;
	}
}
