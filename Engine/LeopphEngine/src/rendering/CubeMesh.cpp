#include "CubeMesh.hpp"

#include "GlContext.hpp"


namespace leopph
{
	CubeMesh::CubeMesh()
	{
		glCreateBuffers(1, &mVbo);
		glNamedBufferStorage(mVbo, sVertices.size() * sizeof(decltype(sVertices)::value_type), sVertices.data(), 0);

		glCreateBuffers(1, &mIbo);
		glNamedBufferStorage(mIbo, sIndices.size() * sizeof(decltype(sIndices)::value_type), sIndices.data(), 0);

		glCreateVertexArrays(1, &mVao);
		glVertexArrayVertexBuffer(mVao, 0, mVbo, 0, 3 * sizeof(decltype(sVertices)::value_type));
		glVertexArrayElementBuffer(mVao, mIbo);

		glVertexArrayAttribFormat(mVao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribBinding(mVao, 0, 0);
		glEnableVertexArrayAttrib(mVao, 0);
	}



	CubeMesh::~CubeMesh()
	{
		glDeleteVertexArrays(1, &mVao);
		glDeleteBuffers(1, &mIbo);
		glDeleteBuffers(1, &mVbo);
	}



	void CubeMesh::draw() const
	{
		glBindVertexArray(mVao);
		glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(sIndices.size()), GL_UNSIGNED_BYTE, nullptr);
	}



	std::vector<f32> const CubeMesh::sVertices
	{
		-1.f, 1.f, 1.f, // 0 left-top-front
		-1.f, -1.f, 1.f, // 1 left-bottom-front
		1.f, 1.f, 1.f, // 2 right-top-front
		1.f, -1.f, 1.f, // 3 right-bottom-front
		1.f, 1.f, -1.f, // 4 right-top-back
		1.f, -1.f, -1.f, // 5 right-bottom-back
		-1.f, 1.f, -1.f, // 6 left-top-back
		-1.f, -1.f, -1.f  // 7 left-bottom-back
	};

	std::vector<u8> const CubeMesh::sIndices
	{
		0u, 1u, 2u, // front upper
		1u, 2u, 3u, // front lower
		2u, 3u, 4u, // right upper
		3u, 4u, 5u, // right lower
		4u, 5u, 6u, // right upper
		5u, 6u, 7u, // right lower
		6u, 7u, 0u, // left upper
		7u, 0u, 1u, // left lower
		0u, 2u, 6u, // top upper
		2u, 6u, 4u, // top lower
		1u, 3u, 5u, // bottom upper
		1u, 5u, 7u  // bottom lower
	};
}
