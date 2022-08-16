#include "PersistentMappedBuffer.hpp"

#include "GlContext.hpp"
#include "Util.hpp"


namespace leopph
{
	PersistentMappedBuffer::PersistentMappedBuffer(u64 const size) :
		mSize{size}

	{
		GLbitfield constexpr mapFlags{GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT};
		auto constexpr createFlags{mapFlags | GL_DYNAMIC_STORAGE_BIT};

		glCreateBuffers(1, &mName);
		glNamedBufferStorage(mName, clamp_cast<GLsizeiptr>(size), nullptr, createFlags);

		mMapping = glMapNamedBufferRange(mName, 0, clamp_cast<GLsizeiptr>(size), mapFlags);
	}



	PersistentMappedBuffer::~PersistentMappedBuffer()
	{
		glUnmapNamedBuffer(mName);
		glDeleteBuffers(1, &mName);
	}



	void* PersistentMappedBuffer::get_ptr() const
	{
		return mMapping;
	}



	u32 PersistentMappedBuffer::get_internal_handle() const
	{
		return mName;
	}



	u64 PersistentMappedBuffer::get_size() const
	{
		return mSize;
	}
}
