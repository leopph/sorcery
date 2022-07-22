#include "windowing/GlWindow.hpp"

#include "EventManager.hpp"
#include "Input.hpp"
#include "InternalContext.hpp"
#include "KeyEvent.hpp"
#include "Logger.hpp"
#include "MouseEvent.hpp"
#include "SettingsImpl.hpp"
#include "WindowEvent.hpp"
#include "rendering/gl/GlCore.hpp"
#include "util/api_adapters/GLFWAdapter.hpp"

#include <array>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	GlWindow::GlWindow() :
		WindowImpl{},
		m_Width{static_cast<int>(GetSettingsImpl()->WindowWidth())},
		m_Height{static_cast<int>(GetSettingsImpl()->WindowHeight())},
		m_Fullscreen{GetSettingsImpl()->Fullscreen()},
		m_Vsync{GetSettingsImpl()->Vsync()},
		m_ClrColor{},
		m_RenderMult{GetSettingsImpl()->RenderMultiplier()}

	{
		if (!glfwInit())
		{
			auto const erroMsg{"Failed to initialize GLFW."};
			Logger::Instance().Critical(erroMsg);
			throw std::runtime_error{erroMsg};
		}

		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);
		Logger::Instance().Debug("Using GLFW " + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev) + ".");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		auto const monitor{m_Fullscreen ? glfwGetPrimaryMonitor() : nullptr};
		m_Window = glfwCreateWindow(m_Width, m_Height, "GlWindow Title", monitor, nullptr);

		if (!m_Window)
		{
			std::string errMsg{"Failed to create GLFW window. "};
			char const* glfwErrMsg;
			glfwGetError(&glfwErrMsg);
			errMsg.append(glfwErrMsg);
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		glfwMakeContextCurrent(m_Window);

		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCursorPosCallback(m_Window, MouseCallback);

		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_FALSE);
		glfwSetWindowAttrib(m_Window, GLFW_AUTO_ICONIFY, GLFW_TRUE);

		glfwSetCursorPos(m_Window, 0, 0);
		glfwSwapInterval(m_Vsync);
		glfwSetWindowUserPointer(m_Window, this);
	}


	GlWindow::~GlWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}


	unsigned GlWindow::Width() const
	{
		return static_cast<int>(m_Width);
	}


	void GlWindow::Width(unsigned const newWidth)
	{
		m_Width = static_cast<int>(newWidth);
		glfwSetWindowSize(m_Window, m_Width, m_Height);
		SendWindowEvent();
	}


	unsigned GlWindow::Height() const
	{
		return static_cast<int>(m_Height);
	}


	void GlWindow::Height(unsigned const newHeight)
	{
		m_Height = static_cast<int>(newHeight);
		glfwSetWindowSize(m_Window, m_Width, m_Height);
		SendWindowEvent();
	}


	bool GlWindow::Fullscreen() const
	{
		return m_Fullscreen;
	}


	void GlWindow::Fullscreen(bool const newValue)
	{
		m_Fullscreen = newValue;

		if (m_Fullscreen)
		{
			glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, m_Width, m_Height, GLFW_DONT_CARE);
			// The requested resolution might not be available in fullscreen mode
			// so we query for the actual resolution we got.
			glfwGetFramebufferSize(m_Window, &m_Width, &m_Height);
		}
		else
		{
			glfwSetWindowMonitor(m_Window, nullptr, 0, 0, m_Width, m_Height, GLFW_DONT_CARE);
			auto const vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowPos(m_Window, static_cast<int>(static_cast<float>(vidMode->width - m_Width) / 2.f), static_cast<int>(static_cast<float>(vidMode->height - m_Height) / 2.f));
		}
		SendWindowEvent();
	}


	bool GlWindow::Vsync() const
	{
		return m_Vsync;
	}


	void GlWindow::Vsync(bool const newValue)
	{
		m_Vsync = newValue;
		glfwSwapInterval(m_Vsync);
		SendWindowEvent();
	}


	std::string_view GlWindow::Title() const
	{
		return m_Title;
	}


	void GlWindow::Title(std::string newTitle)
	{
		m_Title = std::move(newTitle);
		glfwSetWindowTitle(m_Window, m_Title.c_str());
	}


	Vector4 const& GlWindow::ClearColor() const
	{
		return m_ClrColor;
	}


	void GlWindow::ClearColor(Vector4 const& color)
	{
		m_ClrColor = color;
	}


	CursorState GlWindow::CursorMode() const
	{
		return glfw::GetAbstractCursorState(glfwGetInputMode(m_Window, GLFW_CURSOR));
	}


	void GlWindow::CursorMode(CursorState const newState)
	{
		glfwSetInputMode(m_Window, GLFW_CURSOR, glfw::GetGlfwCursorState(newState));
	}


	float GlWindow::RenderMultiplier() const
	{
		return m_RenderMult;
	}


	void GlWindow::RenderMultiplier(float const newMult)
	{
		m_RenderMult = newMult;
		SendWindowEvent();
	}


	void GlWindow::PollEvents()
	{
		glfwPollEvents();
	}


	void GlWindow::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}


	bool GlWindow::ShouldClose()
	{
		return glfwWindowShouldClose(m_Window);
	}


	void GlWindow::ShouldClose(bool const val)
	{
		glfwSetWindowShouldClose(m_Window, val);
	}


	std::vector<GlWindow::DisplayMode> GlWindow::GetSupportedDisplayModes() const
	{
		auto const currentMonitor = glfwGetWindowMonitor(m_Window);
		int numVidModes;
		auto const vidModes = glfwGetVideoModes(currentMonitor ? currentMonitor : glfwGetPrimaryMonitor(), &numVidModes);

		std::vector<DisplayMode> ret;
		ret.reserve(numVidModes);
		// GLFW returns video modes in ascending order and we need them in descending.
		for (auto i = numVidModes - 1; i >= 0; i--)
		{
			ret.emplace_back(vidModes[i].width, vidModes[i].height, vidModes[i].refreshRate);
		}
		return ret;
	}


	void GlWindow::Clear()
	{
		glClearNamedFramebufferfv(0, GL_COLOR, 0, m_ClrColor.Data().data());
		glClearNamedFramebufferfv(0, GL_DEPTH, 0, std::array{1.f}.data());
	}


	void GlWindow::SendWindowEvent() const
	{
		EventManager::Instance().Send<WindowEvent>(static_cast<unsigned>(m_Width), static_cast<unsigned>(m_Height), m_RenderMult, m_Fullscreen, m_Vsync);
	}


	void GlWindow::FramebufferSizeCallback(GLFWwindow* const, int const width, int const height)
	{
		glViewport(0, 0, width, height);
	}


	void GlWindow::KeyCallback(GLFWwindow*, int const key, int, int const action, int)
	{
		try
		{
			auto const keyCode = glfw::GetAbstractKeyCode(key);
			auto const keyState = glfw::GetAbstractKeyState(action);
			EventManager::Instance().Send<KeyEvent>(keyCode, keyState);
		}
		catch (std::out_of_range const&)
		{
			Logger::Instance().Warning("Invalid key input detected.");
		}
	}


	void GlWindow::MouseCallback(GLFWwindow*, double const x, double const y)
	{
		EventManager::Instance().Send<MouseEvent>(Vector2{x, y});
	}
}
