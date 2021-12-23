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

	auto UniformBuffer::Name() const -> unsigned
	{
		return m_Name;
	}

	auto UniformBuffer::Size() const -> std::size_t
	{
		return m_Size;
	}

	auto UniformBuffer::Bind(const int bindingIndex, const int offset, const std::size_t size) const -> void
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, m_Name, offset, size);
	}

	auto UniformBuffer::Store(const int i, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int), &i);
	}

	auto UniformBuffer::Store(const unsigned u, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(u), &u);
	}

	auto UniformBuffer::Store(const float f, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(f), &f);
	}

	auto UniformBuffer::Store(const Vector3& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(vec), vec.Data().data());
	}

	auto UniformBuffer::Store(const Matrix4& mat, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(mat), mat.Data().data());
	}

	auto UniformBuffer::Store(const std::vector<int>& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int) * vec.size(), vec.data());
	}

	auto UniformBuffer::Store(const std::vector<unsigned>& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(unsigned) * vec.size(), vec.data());
	}

	auto UniformBuffer::Store(const std::vector<float>& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(float) * vec.size(), vec.data());
	}

	auto UniformBuffer::Store(const std::vector<Vector3>& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Vector3) * vec.size(), vec.data());
	}

	auto UniformBuffer::Store(const std::vector<Matrix4>& vec, const std::size_t offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Matrix4) * vec.size(), vec.data());
	}
}
