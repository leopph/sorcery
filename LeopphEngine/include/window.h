#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace leopph
{
	class Window
	{
	public:
		static Window& CreateWindow(unsigned width, unsigned height, const std::string& title, bool fullscreen);

		~Window();

		void PollEvents();

	private:
		static Window* s_Instance;

		Window(unsigned width, unsigned height, const std::string& title, bool fullscreen);

		GLFWwindow* m_Window;

		unsigned m_Width;
		unsigned m_Height;
		bool m_Fullscreen;
	};
}