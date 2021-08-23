#pragma once

namespace leopph::impl
{
	class RefCountedBuffer
	{
	public:
		RefCountedBuffer();

		RefCountedBuffer(const RefCountedBuffer& other);
		RefCountedBuffer(RefCountedBuffer&& other) noexcept;

		RefCountedBuffer& operator=(const RefCountedBuffer& other);
		RefCountedBuffer& operator=(RefCountedBuffer&& other) noexcept;

		~RefCountedBuffer();

		const unsigned& name;


	private:
		unsigned m_Name;
	};
}