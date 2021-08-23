#include "RefCountedBuffer.hpp"

#include "../../instances/InstanceHolder.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	RefCountedBuffer::RefCountedBuffer() :
		name{ m_Name }, m_Name{}
	{
		glGenBuffers(1, &m_Name);
		InstanceHolder::RegisterBuffer(*this);
	}


	RefCountedBuffer::RefCountedBuffer(const RefCountedBuffer& other) :
		name { m_Name }, m_Name { other.m_Name }
	{
		InstanceHolder::RegisterBuffer(*this);
	}


	RefCountedBuffer::RefCountedBuffer(RefCountedBuffer&& other) noexcept :
		name { m_Name }, m_Name{ other.m_Name }
	{
		other.m_Name = 0;
	}


	RefCountedBuffer& RefCountedBuffer::operator=(const RefCountedBuffer& other)
	{
		if (m_Name != 0)
		{
			InstanceHolder::UnregisterBuffer(*this);
			if (InstanceHolder::ReferenceCount(*this) == 0)
			{
				glDeleteBuffers(1, &m_Name);
			}
		}

		m_Name = other.m_Name;
		InstanceHolder::RegisterBuffer(*this);
		return *this;
	}


	RefCountedBuffer& RefCountedBuffer::operator=(RefCountedBuffer&& other) noexcept
	{
		if (m_Name != 0)
		{
			InstanceHolder::UnregisterBuffer(*this);
			if (InstanceHolder::ReferenceCount(*this) == 0)
			{
				glDeleteBuffers(1, &m_Name);
			}
		}

		m_Name = other.m_Name;
		other.m_Name = 0;
		return *this;
	}


	RefCountedBuffer::~RefCountedBuffer()
	{
		if (m_Name != 0)
		{
			InstanceHolder::UnregisterBuffer(*this);
			if (InstanceHolder::ReferenceCount(*this) == 0)
			{
				glDeleteBuffers(1, &m_Name);
			}
		}
	}
}
