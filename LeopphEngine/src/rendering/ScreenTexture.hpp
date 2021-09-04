#pragma once

#include <array>


namespace leopph::impl
{
	class ScreenTexture
	{
	public:
		ScreenTexture();

		ScreenTexture(const ScreenTexture& other) = delete;
		ScreenTexture(ScreenTexture&& other) = delete;

		~ScreenTexture();

		ScreenTexture& operator=(const ScreenTexture& other) = delete;
		ScreenTexture& operator=(ScreenTexture&& other) = delete;

		void Draw() const;


	private:
		unsigned m_Vao;
		unsigned m_Vbo;

		static const std::array<float, 20> s_QuadVertices;
	};
}