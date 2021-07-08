#include "glwindow.h"

#include "../components/camera.h"
#include "../input/input.h"
#include "../input/inputhandler.h"

#include "../util/logger.h"

#include <glad/glad.h>
#include <map>
#include <utility>
#include <stdexcept>


namespace leopph::impl
{
	const std::unordered_map<int, KeyCode> GLWindowImpl::s_KeyCodes
	{
		{ GLFW_KEY_0, KeyCode::ZERO },
		{ GLFW_KEY_1, KeyCode::ONE },
		{ GLFW_KEY_2, KeyCode::TWO },
		{ GLFW_KEY_3, KeyCode::THREE },
		{ GLFW_KEY_4, KeyCode::FOUR },
		{ GLFW_KEY_5, KeyCode::FIVE },
		{ GLFW_KEY_6, KeyCode::SIX },
		{ GLFW_KEY_7, KeyCode::SEVEN },
		{ GLFW_KEY_8, KeyCode::EIGHT },
		{ GLFW_KEY_9, KeyCode::NINE },
		{ GLFW_KEY_Q, KeyCode::Q },
		{ GLFW_KEY_W, KeyCode::W },
		{ GLFW_KEY_E, KeyCode::E },
		{ GLFW_KEY_R, KeyCode::R },
		{ GLFW_KEY_T, KeyCode::T },
		{ GLFW_KEY_Y, KeyCode::Y },
		{ GLFW_KEY_U, KeyCode::U },
		{ GLFW_KEY_I, KeyCode::I },
		{ GLFW_KEY_O, KeyCode::O },
		{ GLFW_KEY_P, KeyCode::P },
		{ GLFW_KEY_A, KeyCode::A },
		{ GLFW_KEY_S, KeyCode::S },
		{ GLFW_KEY_D, KeyCode::D },
		{ GLFW_KEY_F, KeyCode::F },
		{ GLFW_KEY_G, KeyCode::G },
		{ GLFW_KEY_H, KeyCode::H },
		{ GLFW_KEY_J, KeyCode::J },
		{ GLFW_KEY_K, KeyCode::K },
		{ GLFW_KEY_L, KeyCode::L },
		{ GLFW_KEY_Z, KeyCode::Z },
		{ GLFW_KEY_X, KeyCode::X },
		{ GLFW_KEY_C, KeyCode::C },
		{ GLFW_KEY_V, KeyCode::V },
		{ GLFW_KEY_B, KeyCode::B },
		{ GLFW_KEY_N, KeyCode::N },
		{ GLFW_KEY_M, KeyCode::M }
	};

	const std::unordered_map<int, KeyState> GLWindowImpl::s_KeyStates
	{
		{ GLFW_PRESS, KeyState::Down },
		{ GLFW_REPEAT, KeyState::Held },
		{ GLFW_RELEASE, KeyState::Up }
	};

	

	GLWindowImpl::GLWindowImpl(unsigned width, unsigned height, const std::string& title, bool fullscreen)
		: Window{ width, height, title, fullscreen }, m_Vsync{ false }
	{
		if (!glfwInit())
		{
			const auto erroMsg{ "Failed to initialize GLFW." };
			Logger::Instance().Error(erroMsg);
			throw std::exception{ erroMsg };
		}

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWmonitor* monitor{ nullptr };

		if (Fullscreen())
			monitor = glfwGetPrimaryMonitor();

		m_Window = glfwCreateWindow(static_cast<int>(width), static_cast<int>(height), title.data(), monitor, nullptr);

		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCursorPosCallback(m_Window, MouseCallback);
		glfwSetCursorPos(m_Window, 0, 0);
		glfwSwapInterval(0);
	}
	
	GLWindowImpl::~GLWindowImpl()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}


	
	void GLWindowImpl::InitKeys()
	{
		for (const auto& pair : s_KeyCodes)
			InputHandler::OnInputChange(pair.second, KeyState::Released);
	}


#pragma warning(push)
#pragma warning(disable: 4100)
	void GLWindowImpl::FramebufferSizeCallback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		auto& windowInstance{ Window::Get() };
		windowInstance.Width(width);
		windowInstance.Height(height);

		if (Camera::Active() != nullptr)
			Camera::Active()->AspectRatio(width, height);
	}

	void GLWindowImpl::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		try
		{
			const KeyCode keyCode = s_KeyCodes.at(key);
			const KeyState keyState = s_KeyStates.at(action);	
			InputHandler::OnInputChange(keyCode, keyState);
		}
		catch (const std::out_of_range&)
		{
			Logger::Instance().Warning("Invalid key input detected.");
		}
	}
	
	void GLWindowImpl::MouseCallback(GLFWwindow* window, double x, double y)
	{
		InputHandler::OnInputChange(x, y);
	}
#pragma warning(pop)



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

	void GLWindowImpl::Vsync(bool value)
	{
		if (value)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
	}

	bool GLWindowImpl::Vsync() const
	{
		return m_Vsync;
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
		Vector3 color{ Background() };
		glClearColor(color[0], color[1], color[2], 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
}