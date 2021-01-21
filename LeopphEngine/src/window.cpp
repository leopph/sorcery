#include "window.h"

#include <stdexcept>

namespace leopph
{
	Window* Window::s_Instance{ nullptr };

	Window& Window::CreateWindow(unsigned width = 1280u, unsigned height = 720u, const std::string& title = "Window", bool fullscreen = false)
	{
		if (s_Instance == nullptr)
			s_Instance = new Window(width, height, title, fullscreen);

		return *s_Instance;
	}

	Window::Window(unsigned width, unsigned height, const std::string& title, bool fullscreen)
		: m_Width{ width }, m_Height{ height }, m_Fullscreen{ fullscreen }
	{
		if (!glfwInit())
			throw std::exception{};

		GLFWmonitor* monitor{ nullptr };

		if (m_Fullscreen)
			monitor = glfwGetPrimaryMonitor();

		m_Window = glfwCreateWindow(m_Width, m_Height, title.data(), monitor, nullptr);
	}

	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}
}