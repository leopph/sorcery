#include "glwindow.h"
#include "../components/camera.h"
#include "../input/input.h"

#include <stdexcept>
#include <glad/glad.h>
#include <utility>
#include <map>


namespace leopph::impl
{
	// init static members
	std::function<void(int, int)> GLWindowImpl::s_KeyCallback{};
	std::function<void(float, float)> GLWindowImpl::s_MouseCallback{};


	// constructor
	GLWindowImpl::GLWindowImpl(unsigned width, unsigned height, const std::string& title, bool fullscreen)
		: Window{ width, height, title, fullscreen }
	{
		if (!glfwInit())
			throw std::exception{};

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWmonitor* monitor{ nullptr };

		if (Fullscreen())
			monitor = glfwGetPrimaryMonitor();

		m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.data(), monitor, nullptr);

		glfwMakeContextCurrent(m_Window);
		Input::RegisterCallbacks();
		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallbackManager);
		glfwSetCursorPosCallback(m_Window, MouseCallbackManager);
		glfwSetCursorPos(m_Window, 0, 0);
	}




	// destructor
	GLWindowImpl::~GLWindowImpl()
	{
		glfwDestroyWindow(m_Window);
	}





#pragma warning(push)
#pragma warning(disable: 4100)
	// framebuffer resize callback
	void GLWindowImpl::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		auto& windowInstance{ Window::Get() };
		windowInstance.Width(width);
		windowInstance.Height(height);

		if (Camera::Active() != nullptr)
			Camera::Active()->AspectRatio(width, height);
	}





	void GLWindowImpl::KeyCallbackManager(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (s_KeyCallback)
			s_KeyCallback(key, action);
	}
#pragma warning(pop)


	void GLWindowImpl::SetKeyCallback(std::function<void(int, int)> callback)
	{
		s_KeyCallback = std::move(callback);
	}


#pragma warning(push)
#pragma warning(disable: 4100)
	void GLWindowImpl::MouseCallbackManager(GLFWwindow* window, double x, double y)
	{
		if (s_MouseCallback)
			s_MouseCallback(static_cast<float>(x), static_cast<float>(y));
	}
#pragma warning(pop)

	void GLWindowImpl::SetMouseCallback(std::function<void(float, float)> callback)
	{
		s_MouseCallback = std::move(callback);
	}


	void GLWindowImpl::Width(unsigned newWidth)
	{
		Window::Width(newWidth);
		glfwSetWindowSize(m_Window, static_cast<int>(newWidth), static_cast<int>(Window::Height()));
	}

	void GLWindowImpl::Height(unsigned newHeight)
	{
		Window::Height(newHeight);
		glfwSetWindowSize(m_Window, static_cast<int>(Window::Width()), static_cast<int>(newHeight));
	}


	void GLWindowImpl::PollEvents()
	{
		glfwPollEvents();
	}

	void GLWindowImpl::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

	bool GLWindowImpl::ShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}

	void GLWindowImpl::Clear()
	{
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}


	CursorState GLWindowImpl::CursorMode() const
	{
		static const std::map<decltype(GLFW_CURSOR_NORMAL), CursorState> cursorStates
		{
			{ GLFW_CURSOR_NORMAL, CursorState::Shown },
			{ GLFW_CURSOR_HIDDEN, CursorState::Hidden },
			{ GLFW_CURSOR_DISABLED, CursorState::Disabled }
		};

		return cursorStates.at(glfwGetInputMode(this->m_Window, GLFW_CURSOR));
	}

	void GLWindowImpl::CursorMode(CursorState newState)
	{
		static const std::map<CursorState, decltype(GLFW_CURSOR_NORMAL)> cursorStates
		{
			{ CursorState::Shown, GLFW_CURSOR_NORMAL },
			{ CursorState::Hidden, GLFW_CURSOR_HIDDEN },
			{ CursorState::Disabled, GLFW_CURSOR_DISABLED }
		};

		glfwSetInputMode(this->m_Window, GLFW_CURSOR, cursorStates.at(newState));
	}
}