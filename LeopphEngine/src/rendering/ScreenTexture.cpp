#include "ScreenTexture.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	ScreenTexture::ScreenTexture() :
		m_Vao{}, m_Vbo{}
	{
		glCreateBuffers(1, &m_Vbo);
		glNamedBufferStorage(m_Vbo, s_QuadVertices.size() * sizeof(decltype(s_QuadVertices)::value_type), s_QuadVertices.data(), 0);

		glCreateVertexArrays(1, &m_Vao);
		glVertexArrayVertexBuffer(m_Vao, 0, m_Vbo, 0, 5 * sizeof(decltype(s_QuadVertices)::value_type));

		glEnableVertexArrayAttrib(m_Vao, 0);
		glEnableVertexArrayAttrib(m_Vao, 1);

		glVertexArrayAttribFormat(m_Vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_Vao, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(decltype(s_QuadVertices)::value_type));

		glVertexArrayAttribBinding(m_Vao, 0, 0);
		glVertexArrayAttribBinding(m_Vao, 1, 0);
	}


	ScreenTexture::~ScreenTexture()
	{
		glDeleteBuffers(1, &m_Vbo);
		glDeleteVertexArrays(1, &m_Vao);
	}


	void ScreenTexture::Draw() const
	{
		glBindVertexArray(m_Vao);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}


	const std::array<float, 20> ScreenTexture::s_QuadVertices
	{
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
	};
}