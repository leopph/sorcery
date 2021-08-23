#pragma once

namespace leopph::impl
{
	class Buffer
	{
	public:
		Buffer();

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&&) = delete;

		Buffer& operator=(const Buffer&) = delete;
		Buffer& operator=(Buffer&&) = delete;

		~Buffer();

		const unsigned& name;

	 
	private:
		unsigned m_Name;
	};
}