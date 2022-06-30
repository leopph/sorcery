#include "rendering/ScreenQuad.hpp"


namespace leopph::internal
{
	ScreenQuad::ScreenQuad()
	{
		glNamedBufferStorage(m_VertexBuffer, QUAD_VERTICES.size() * sizeof(decltype(QUAD_VERTICES)::value_type), QUAD_VERTICES.data(), 0);

		glVertexArrayVertexBuffer(m_VertexArray, 0, m_VertexBuffer, 0, 5 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glEnableVertexArrayAttrib(m_VertexArray, 0);
		glEnableVertexArrayAttrib(m_VertexArray, 1);

		glVertexArrayAttribFormat(m_VertexArray, 0, 3, GL_FLOAT, GL_FALSE, 0);
		glVertexArrayAttribFormat(m_VertexArray, 1, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(decltype(QUAD_VERTICES)::value_type));

		glVertexArrayAttribBinding(m_VertexArray, 0, 0);
		glVertexArrayAttribBinding(m_VertexArray, 1, 0);
	}


	auto ScreenQuad::Draw() const -> void
	{
		glBindVertexArray(m_VertexArray);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}
}
