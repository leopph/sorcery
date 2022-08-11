#include "Window.hpp"

#include "EventManager.hpp"
#include "KeyEvent.hpp"
#include "Logger.hpp"
#include "MouseEvent.hpp"
#include "SettingsImpl.hpp"
#include "WindowEvent.hpp"
#include "util/api_adapters/GLFWAdapter.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>


namespace leopph
{
	u32 Window::get_width() const
	{
		return mWidth;
	}



	void Window::set_width(u32 const width)
	{
		mWidth = static_cast<int>(width);
		glfwSetWindowSize(mHandle, mWidth, mHeight);
		send_window_event();
	}



	u32 Window::get_height() const
	{
		return mHeight;
	}



	void Window::set_height(u32 const height)
	{
		mHeight = static_cast<int>(height);
		glfwSetWindowSize(mHandle, mWidth, mHeight);
		send_window_event();
	}



	f32 Window::get_aspect_ratio() const
	{
		return static_cast<f32>(mWidth) / static_cast<f32>(mHeight);
	}



	bool Window::is_fullscreen() const
	{
		return mFullscreen;
	}



	void Window::set_fullscreen(bool const fullscreen)
	{
		mFullscreen = fullscreen;

		if (mFullscreen)
		{
			glfwSetWindowMonitor(mHandle, glfwGetPrimaryMonitor(), 0, 0, mWidth, mHeight, GLFW_DONT_CARE);
			// The requested resolution might not be available in fullscreen mode
			// so we query for the actual resolution we got.
			glfwGetFramebufferSize(mHandle, &mWidth, &mHeight);
		}
		else
		{
			glfwSetWindowMonitor(mHandle, nullptr, 0, 0, mWidth, mHeight, GLFW_DONT_CARE);
			auto const vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowPos(mHandle, static_cast<int>(static_cast<float>(vidMode->width - mWidth) / 2.f), static_cast<int>(static_cast<float>(vidMode->height - mHeight) / 2.f));
		}

		send_window_event();
	}



	bool Window::get_vsync() const
	{
		return mVsync;
	}



	void Window::set_vsync(bool const vsync)
	{
		mVsync = vsync;
		glfwSwapInterval(mVsync);
		send_window_event();
	}



	std::string_view Window::get_title() const
	{
		return mTitle;
	}



	void Window::set_title(std::string title)
	{
		mTitle = std::move(title);
		glfwSetWindowTitle(mHandle, mTitle.c_str());
	}



	CursorState Window::get_cursor_state() const
	{
		return internal::glfw::GetAbstractCursorState(glfwGetInputMode(mHandle, GLFW_CURSOR));
	}



	void Window::set_cursor_state(CursorState const cursorState)
	{
		glfwSetInputMode(mHandle, GLFW_CURSOR, internal::glfw::GetGlfwCursorState(cursorState));
	}



	std::vector<Window::DisplayMode> Window::get_supported_display_modes() const
	{
		auto const currentMonitor = glfwGetWindowMonitor(mHandle);
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



	bool Window::should_close() const
	{
		return glfwWindowShouldClose(mHandle);
	}



	void Window::set_should_close(bool const shouldClose)
	{
		glfwSetWindowShouldClose(mHandle, shouldClose);
	}



	void Window::poll_events()
	{
		glfwPollEvents();
	}



	void Window::swap_buffers()
	{
		glfwSwapBuffers(mHandle);
	}



	Window::Window()
	{
		if (!glfwInit())
		{
			auto const errMsg = "Failed to initialize GLFW.";
			internal::Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);
		internal::Logger::Instance().Debug(std::format("GLFW version {}.{}.{}.", major, minor, rev));

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

		mHandle = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

		if (!mHandle)
		{
			char const* glfwErrMsg;
			glfwGetError(&glfwErrMsg);

			auto const errMsg = std::format("Error while creating window: {}.", glfwErrMsg);
			internal::Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		glfwMakeContextCurrent(mHandle);

		glfwSetWindowUserPointer(mHandle, this);
		glfwSetFramebufferSizeCallback(mHandle, framebuffer_size_callback);
		glfwSetKeyCallback(mHandle, key_callback);
		glfwSetCursorPosCallback(mHandle, mouse_callback);

		glfwSetCursorPos(mHandle, 0, 0);
		glfwSwapInterval(mVsync);
	}



	Window::~Window()
	{
		glfwDestroyWindow(mHandle);
	}



	void Window::send_window_event() const
	{
		EventManager::Instance().Send<internal::WindowEvent>(static_cast<u32>(mWidth), static_cast<u32>(mHeight), mFullscreen, mVsync);
	}



	void Window::framebuffer_size_callback(GLFWwindow* glfwWindow, int const width, int const height)
	{
		auto* const window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
		window->mWidth = width;
		window->mHeight = height;
	}



	void Window::key_callback(GLFWwindow*, int const key, int, int const action, int)
	{
		try
		{
			auto const keyCode = internal::glfw::GetAbstractKeyCode(key);
			auto const keyState = internal::glfw::GetAbstractKeyState(action);
			EventManager::Instance().Send<internal::KeyEvent>(keyCode, keyState);
		}
		catch (std::out_of_range const&)
		{
			internal::Logger::Instance().Warning("Invalid key input detected.");
		}
	}



	void Window::mouse_callback(GLFWwindow*, double const x, double const y)
	{
		EventManager::Instance().Send<internal::MouseEvent>(Vector2{x, y});
	}
}
