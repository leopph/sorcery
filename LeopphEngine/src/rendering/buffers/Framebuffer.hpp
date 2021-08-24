#pragma once

namespace leopph::impl
{
	class Framebuffer
	{
	public:
		Framebuffer();

		Framebuffer(const Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) = delete;

		Framebuffer& operator=(const Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = delete;

		virtual ~Framebuffer();

		const unsigned& name;


	private:
		unsigned m_Name;
	};
}