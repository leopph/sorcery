#include "UniformBuffer.hpp"

#include <glad/glad.h>


namespace leopph::internal
{
	UniformBuffer::UniformBuffer() :
		m_Name{},
		m_Size{}
	{
		glCreateBuffers(1, &m_Name);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_Name);
	}

	unsigned UniformBuffer::Name() const
	{
		return m_Name;
	}

	std::size_t UniformBuffer::Size() const
	{
		return m_Size;
	}

	void UniformBuffer::Bind(const int bindingIndex, const int offset, const std::size_t size) const
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, m_Name, offset, size);
	}

	void UniformBuffer::Store(const int i, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int), &i);
	}

	void UniformBuffer::Store(const unsigned u, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(u), &u);
	}

	void UniformBuffer::Store(const float f, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(f), &f);
	}

	void UniformBuffer::Store(const Vector3& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(vec), vec.Data().data());
	}

	void UniformBuffer::Store(const Matrix4& mat, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(mat), mat.Data().data());
	}

	void UniformBuffer::Store(const std::vector<int>& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int) * vec.size(), vec.data());
	}

	void UniformBuffer::Store(const std::vector<unsigned>& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(unsigned) * vec.size(), vec.data());
	}

	void UniformBuffer::Store(const std::vector<float>& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(float) * vec.size(), vec.data());
	}

	void UniformBuffer::Store(const std::vector<Vector3>& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Vector3) * vec.size(), vec.data());
	}

	void UniformBuffer::Store(const std::vector<Matrix4>& vec, const std::size_t offset) const
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Matrix4) * vec.size(), vec.data());
	}

}