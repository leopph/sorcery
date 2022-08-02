#include "QuadMesh.hpp"

#include "GlCore.hpp"


namespace leopph
{
	QuadMesh::QuadMesh()
	{}



	QuadMesh::~QuadMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}



	void QuadMesh::draw() const
	{
		glBindVertexArray(mVao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sIndices.size()), GL_UNSIGNED_BYTE, nullptr);
	}



	std::vector<f32> const QuadMesh::sVertices
	{
		-1, 1, 0,
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0
	};

	std::vector<u8> const QuadMesh::sIndices
	{
		0, 1, 2,
		2, 3, 0
	};
}
