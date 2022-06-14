#include "UniformBuffer.hpp"

#include "opengl/OpenGl.hpp"


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


	auto UniformBuffer::Bind(int const bindingIndex, int const offset, std::size_t const size) const -> void
	{
		glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, m_Name, offset, size);
	}


	auto UniformBuffer::Store(int const i, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int), &i);
	}


	auto UniformBuffer::Store(unsigned const u, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(u), &u);
	}


	auto UniformBuffer::Store(float const f, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(f), &f);
	}


	auto UniformBuffer::Store(Vector3 const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(vec), vec.Data().data());
	}


	auto UniformBuffer::Store(Matrix4 const& mat, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(mat), mat.Data().data());
	}


	auto UniformBuffer::Store(std::vector<int> const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(int) * vec.size(), vec.data());
	}


	auto UniformBuffer::Store(std::vector<unsigned> const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(unsigned) * vec.size(), vec.data());
	}


	auto UniformBuffer::Store(std::vector<float> const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(float) * vec.size(), vec.data());
	}


	auto UniformBuffer::Store(std::vector<Vector3> const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Vector3) * vec.size(), vec.data());
	}


	auto UniformBuffer::Store(std::vector<Matrix4> const& vec, std::size_t const offset) const -> void
	{
		glNamedBufferSubData(m_Name, offset, sizeof(Matrix4) * vec.size(), vec.data());
	}
}
