#include "GlWindow.hpp"

#include "../config/Settings.hpp"
#include "../events/KeyEvent.hpp"
#include "../events/MouseEvent.hpp"
#include "../events/WindowEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../input/Input.hpp"
#include "../util/Logger.hpp"
#include "../util/api_adapters/GLFWAdapter.hpp"

#include <glad/gl.h>

#include <array>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	GlWindow::GlWindow() :
		WindowImpl{},
		m_Width{static_cast<int>(Settings::Instance().WindowWidth())},
		m_Height{static_cast<int>(Settings::Instance().WindowHeight())},
		m_Fullscreen{Settings::Instance().Fullscreen()},
		m_Vsync{Settings::Instance().Vsync()},
		m_ClrColor{},
		m_RenderMult{Settings::Instance().RenderMultiplier()}

	{
		if (!glfwInit())
		{
			const auto erroMsg{"Failed to initialize GLFW."};
			Logger::Instance().Critical(erroMsg);
			throw std::runtime_error{erroMsg};
		}

		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);
		Logger::Instance().Debug("Using GLFW " + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev) + ".");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		const auto monitor{m_Fullscreen ? glfwGetPrimaryMonitor() : nullptr};
		m_Window = glfwCreateWindow(m_Width, m_Height, "GlWindow Title", monitor, nullptr);

		if (!m_Window)
		{
			const auto errMsg{"Failed to create GLFW window."};
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		glfwMakeContextCurrent(m_Window);

		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCursorPosCallback(m_Window, MouseCallback);

		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_FALSE);
		glfwSetWindowAttrib(m_Window, GLFW_AUTO_ICONIFY, GLFW_FALSE);

		glfwSetCursorPos(m_Window, 0, 0);
		glfwSwapInterval(m_Vsync);
		glfwSetWindowUserPointer(m_Window, this);
	}


	GlWindow::~GlWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}


	auto GlWindow::Width() const -> unsigned
	{
		return static_cast<int>(m_Width);
	}


	auto GlWindow::Width(const unsigned newWidth) -> void
	{
		m_Width = static_cast<int>(newWidth);
		glfwSetWindowSize(m_Window, m_Width, m_Height);
	}


	auto GlWindow::Height() const -> unsigned
	{
		return static_cast<int>(m_Height);
	}


	auto GlWindow::Height(const unsigned newHeight) -> void
	{
		m_Height = static_cast<int>(newHeight);
		glfwSetWindowSize(m_Window, m_Width, m_Height);
	}


	auto GlWindow::Fullscreen() const -> bool
	{
		return m_Fullscreen;
	}


	auto GlWindow::Fullscreen(const bool newValue) -> void
	{
		m_Fullscreen = newValue;
		const auto monitor{newValue ? glfwGetPrimaryMonitor() : nullptr};
		glfwSetWindowMonitor(m_Window, monitor, 0, 0, m_Width, m_Height, GLFW_DONT_CARE);
	}


	auto GlWindow::Vsync() const -> bool
	{
		return m_Vsync;
	}


	auto GlWindow::Vsync(const bool newValue) -> void
	{
		m_Vsync = newValue;
		glfwSwapInterval(m_Vsync ? 1 : 0);
	}


	auto GlWindow::Title() const -> std::string_view
	{
		return m_Title;
	}


	auto GlWindow::Title(std::string newTitle) -> void
	{
		m_Title = std::move(newTitle);
		glfwSetWindowTitle(m_Window, m_Title.c_str());
	}


	auto GlWindow::ClearColor() const -> const Vector4&
	{
		return m_ClrColor;
	}


	auto GlWindow::ClearColor(const Vector4& color) -> void
	{
		m_ClrColor = color;
	}


	auto GlWindow::CursorMode() const -> CursorState
	{
		return glfw::GetAbstractCursorState(glfwGetInputMode(m_Window, GLFW_CURSOR));
	}


	auto GlWindow::CursorMode(const CursorState newState) -> void
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, glfw::GetGlfwCursorState(newState));
	}


	auto GlWindow::RenderMultiplier() const -> float
	{
		return m_RenderMult;
	}


	auto GlWindow::RenderMultiplier(const float newMult) -> void
	{
		m_RenderMult = newMult;
		EventManager::Instance().Send<WindowEvent>(Vector2{m_Width, m_Height}, m_RenderMult, m_Vsync, m_Fullscreen);
	}


	auto GlWindow::PollEvents() -> void
	{
		glfwPollEvents();
	}


	auto GlWindow::SwapBuffers() -> void
	{
		glfwSwapBuffers(m_Window);
	}


	auto GlWindow::ShouldClose() -> bool
	{
		return glfwWindowShouldClose(m_Window);
	}


	auto GlWindow::ShouldClose(const bool val) -> void
	{
		glfwSetWindowShouldClose(m_Window, val);
	}


	auto GlWindow::Clear() -> void
	{
		glClearNamedFramebufferfv(0, GL_COLOR, 0, m_ClrColor.Data().data());
		glClearNamedFramebufferfv(0, GL_DEPTH, 0, std::array{1.f}.data());
	}


	auto GlWindow::OnEventReceived(EventParamType event) -> void
	{
		m_Fullscreen = event.Fullscreen;
		m_Vsync = event.Vsync;
		m_RenderMult = event.RenderMultiplier;
		m_Width = static_cast<int>(event.Resolution[0]);
		m_Height = static_cast<int>(event.Resolution[1]);
	}


	auto GlWindow::FramebufferSizeCallback(GLFWwindow* const window, const int width, const int height) -> void
	{
		glViewport(0, 0, width, height);

		const auto instance = static_cast<GlWindow*>(glfwGetWindowUserPointer(window));
		instance->m_Width = width;
		instance->m_Height = height;

		EventManager::Instance().Send<WindowEvent>(Vector2{width, height}, instance->RenderMultiplier(), instance->m_Vsync, instance->m_Fullscreen);
	}


	auto GlWindow::KeyCallback(GLFWwindow*, const int key, int, const int action, int) -> void
	{
		try
		{
			const auto keyCode = glfw::GetAbstractKeyCode(key);
			const auto keyState = glfw::GetAbstractKeyState(action);
			EventManager::Instance().Send<KeyEvent>(keyCode, keyState);
		}
		catch (const std::out_of_range&)
		{
			Logger::Instance().Warning("Invalid key input detected.");
		}
	}


	auto GlWindow::MouseCallback(GLFWwindow*, const double x, const double y) -> void
	{
		EventManager::Instance().Send<MouseEvent>(Vector2{x, y});
	}
}
