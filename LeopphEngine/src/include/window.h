#pragma once

#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <string>
#include <functional>

#include "leopphapi.h"


namespace leopph::implementation
{
	class LEOPPHAPI Window
	{
	public:
		static Window& CreateWindow(unsigned width, unsigned height, const std::string& title, bool fullscreen);
		static void Destroy();

		static void SetKeyCallback(std::function<void(int, int)> callback);
		static void SetMouseCallback(std::function<void(float, float)> callback);

		void PollEvents();
		void SwapBuffers();
		bool ShouldClose();
		void Clear();


	private:
		static Window* s_Instance;
		static std::function<void(int, int)> s_KeyCallback;
		static std::function<void(float, float)> s_MouseCallback;

		Window(unsigned width, unsigned height, const std::string& title, bool fullscreen);
		~Window();

		static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallbackManager(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void MouseCallbackManager(GLFWwindow* window, double x, double y);

		GLFWwindow* m_Window;
		unsigned m_Width;
		unsigned m_Height;
		bool m_Fullscreen;
	};
}