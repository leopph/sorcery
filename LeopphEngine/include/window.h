#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>

#include "leopphapi.h"

namespace leopph
{
	class LEOPPHAPI Window
	{
	public:
		static Window& CreateWindow(unsigned width, unsigned height, const std::string& title, bool fullscreen);

		~Window();

		void PollEvents();
		void SwapBuffers();
		bool ShouldClose();
		void Clear();

	private:
		static Window* s_Instance;

		Window(unsigned width, unsigned height, const std::string& title, bool fullscreen);

		GLFWwindow* m_Window;

		unsigned m_Width;
		unsigned m_Height;
		bool m_Fullscreen;
	};
}