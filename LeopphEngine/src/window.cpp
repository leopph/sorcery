#include "window.h"
#include "camera.h"
#include "input.h"

#include <stdexcept>
#include <glad/glad.h>


namespace leopph::implementation
{
	// init window member
	Window* Window::s_Instance{ nullptr };


	// set up a window and rendering context with callbacks
	Window& Window::CreateWindow(unsigned width = 1280u, unsigned height = 720u, const std::string& title = "Window", bool fullscreen = false)
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new Window(width, height, title, fullscreen);

			glfwMakeContextCurrent(s_Instance->m_Window);

			Camera::Instance().AspectRatio(s_Instance->m_Width, s_Instance->m_Height);

			Input::RegisterWindow(s_Instance->m_Window);

			glfwSetFramebufferSizeCallback(s_Instance->m_Window, FramebufferSizeCallback);
		}

		return *s_Instance;
	}



	// window constructor
	Window::Window(unsigned width, unsigned height, const std::string& title, bool fullscreen)
		: m_Width{ width }, m_Height{ height }, m_Fullscreen{ fullscreen }
	{
		if (!glfwInit())
			throw std::exception{};

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWmonitor* monitor{ nullptr };

		if (m_Fullscreen)
			monitor = glfwGetPrimaryMonitor();

		m_Window = glfwCreateWindow(m_Width, m_Height, title.data(), monitor, nullptr);
	}


	// framebuffer resize callback
	void Window::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		s_Instance->m_Width = width;
		s_Instance->m_Height = height;

		Camera::Instance().AspectRatio(s_Instance->m_Width, s_Instance->m_Height);
	}



	Window::~Window()
	{
		glfwDestroyWindow(m_Window);
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	bool Window::ShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}

	void Window::Clear()
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}