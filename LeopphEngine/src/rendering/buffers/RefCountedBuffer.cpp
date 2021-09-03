#include "RefCountedBuffer.hpp"

#include "../../data/DataManager.hpp"

#include <glad/glad.h>


namespace leopph::impl
{
	RefCountedBuffer::RefCountedBuffer() :
		name{ m_Name }, m_Name{}
	{
		glGenBuffers(1, &m_Name);
		DataManager::Register(*this);
	}


	RefCountedBuffer::RefCountedBuffer(const RefCountedBuffer& other) :
		name { m_Name }, m_Name { other.m_Name }
	{
		DataManager::Register(*this);
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
			DataManager::Unregister(*this);
			if (DataManager::Count(*this) == 0)
			{
				glDeleteBuffers(1, &m_Name);
			}
		}

		m_Name = other.m_Name;
		DataManager::Register(*this);
		return *this;
	}


	RefCountedBuffer& RefCountedBuffer::operator=(RefCountedBuffer&& other) noexcept
	{
		if (m_Name != 0)
		{
			DataManager::Unregister(*this);
			if (DataManager::Count(*this) == 0)
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
			DataManager::Unregister(*this);
			if (DataManager::Count(*this) == 0)
			{
				glDeleteBuffers(1, &m_Name);
			}
		}
	}
}
