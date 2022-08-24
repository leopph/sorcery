#include "Window.hpp"

#include "Bimap.hpp"
#include "Event.hpp"
#include "Input.hpp"
#include "Logger.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <format>
#include <unordered_map>


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
		switch (glfwGetInputMode(mHandle, GLFW_CURSOR))
		{
			case GLFW_CURSOR_DISABLED:
			{
				return CursorState::Disabled;
			}

			case GLFW_CURSOR_HIDDEN:
			{
				return CursorState::Hidden;
			}

			case GLFW_CURSOR_NORMAL:
			default:
			{
				return CursorState::Shown;
			}
		}
	}



	void Window::set_cursor_state(CursorState const cursorState)
	{
		int glfwInputMode{};

		switch (cursorState)
		{
			case CursorState::Disabled:
			{
				glfwInputMode = GLFW_CURSOR_DISABLED;
				break;
			}

			case CursorState::Hidden:
			{
				glfwInputMode = GLFW_CURSOR_HIDDEN;
				break;
			}

			case CursorState::Shown:
			{
				glfwInputMode = GLFW_CURSOR_NORMAL;
				break;
			}
		}

		glfwSetInputMode(mHandle, GLFW_CURSOR, glfwInputMode);
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



	void Window::wait_events()
	{
		glfwWaitEvents();
	}



	void Window::wait_events(f32 const timeout)
	{
		return glfwWaitEventsTimeout(timeout);
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
			Logger::get_instance().critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);
		Logger::get_instance().debug(std::format("GLFW {}.{}.{}", major, minor, rev));

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

		mHandle = glfwCreateWindow(mWidth, mHeight, mTitle.c_str(), nullptr, nullptr);

		if (!mHandle)
		{
			char const* glfwErrMsg;
			glfwGetError(&glfwErrMsg);

			auto const errMsg = std::format("Error while creating window: {}.", glfwErrMsg);
			Logger::get_instance().critical(errMsg);
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
		glfwTerminate();
	}



	void Window::send_window_event() const
	{
		EventManager::get_instance().send<WindowEvent>(static_cast<u32>(mWidth), static_cast<u32>(mHeight), mFullscreen, mVsync);
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
			std::unordered_map<int, KeyCode> const keyCodes
			{
				{GLFW_KEY_0, KeyCode::Zero},
				{GLFW_KEY_1, KeyCode::One},
				{GLFW_KEY_2, KeyCode::Two},
				{GLFW_KEY_3, KeyCode::Three},
				{GLFW_KEY_4, KeyCode::Four},
				{GLFW_KEY_5, KeyCode::Five},
				{GLFW_KEY_6, KeyCode::Six},
				{GLFW_KEY_7, KeyCode::Seven},
				{GLFW_KEY_8, KeyCode::Eight},
				{GLFW_KEY_9, KeyCode::Nine},
				{GLFW_KEY_A, KeyCode::A},
				{GLFW_KEY_B, KeyCode::B},
				{GLFW_KEY_C, KeyCode::C},
				{GLFW_KEY_D, KeyCode::D},
				{GLFW_KEY_E, KeyCode::E},
				{GLFW_KEY_F, KeyCode::F},
				{GLFW_KEY_G, KeyCode::G},
				{GLFW_KEY_H, KeyCode::H},
				{GLFW_KEY_I, KeyCode::I},
				{GLFW_KEY_J, KeyCode::J},
				{GLFW_KEY_K, KeyCode::K},
				{GLFW_KEY_L, KeyCode::L},
				{GLFW_KEY_M, KeyCode::M},
				{GLFW_KEY_N, KeyCode::N},
				{GLFW_KEY_O, KeyCode::O},
				{GLFW_KEY_P, KeyCode::P},
				{GLFW_KEY_Q, KeyCode::Q},
				{GLFW_KEY_R, KeyCode::R},
				{GLFW_KEY_S, KeyCode::S},
				{GLFW_KEY_T, KeyCode::T},
				{GLFW_KEY_U, KeyCode::U},
				{GLFW_KEY_V, KeyCode::V},
				{GLFW_KEY_W, KeyCode::W},
				{GLFW_KEY_X, KeyCode::X},
				{GLFW_KEY_Y, KeyCode::Y},
				{GLFW_KEY_Z, KeyCode::Z},
				{GLFW_KEY_SPACE, KeyCode::Space},
				{GLFW_KEY_APOSTROPHE, KeyCode::Apostrophe},
				{GLFW_KEY_COMMA, KeyCode::Comma},
				{GLFW_KEY_MINUS, KeyCode::Minus},
				{GLFW_KEY_PERIOD, KeyCode::Period},
				{GLFW_KEY_SLASH, KeyCode::Slash},
				{GLFW_KEY_SEMICOLON, KeyCode::Semicolon},
				{GLFW_KEY_EQUAL, KeyCode::Equal},
				{GLFW_KEY_LEFT_BRACKET, KeyCode::LeftBracket},
				{GLFW_KEY_BACKSLASH, KeyCode::Backslash},
				{GLFW_KEY_RIGHT_BRACKET, KeyCode::RightBracket},
				{GLFW_KEY_GRAVE_ACCENT, KeyCode::GraveAccent},
				{GLFW_KEY_ESCAPE, KeyCode::Escape},
				{GLFW_KEY_ENTER, KeyCode::Enter,},
				{GLFW_KEY_TAB, KeyCode::Tab},
				{GLFW_KEY_BACKSPACE, KeyCode::Backspace},
				{GLFW_KEY_INSERT, KeyCode::Insert},
				{GLFW_KEY_DELETE, KeyCode::Delete},
				{GLFW_KEY_RIGHT, KeyCode::Right},
				{GLFW_KEY_LEFT, KeyCode::Left},
				{GLFW_KEY_DOWN, KeyCode::Down},
				{GLFW_KEY_UP, KeyCode::Up},
				{GLFW_KEY_PAGE_UP, KeyCode::PageUp},
				{GLFW_KEY_PAGE_DOWN, KeyCode::PageDown},
				{GLFW_KEY_HOME, KeyCode::Home},
				{GLFW_KEY_END, KeyCode::End},
				{GLFW_KEY_CAPS_LOCK, KeyCode::CapsLock},
				{GLFW_KEY_SCROLL_LOCK, KeyCode::ScrollLock},
				{GLFW_KEY_NUM_LOCK, KeyCode::NumLock},
				{GLFW_KEY_PRINT_SCREEN, KeyCode::PrintScreen},
				{GLFW_KEY_PAUSE, KeyCode::Pause},
				{GLFW_KEY_F1, KeyCode::F1},
				{GLFW_KEY_F2, KeyCode::F2},
				{GLFW_KEY_F3, KeyCode::F3},
				{GLFW_KEY_F4, KeyCode::F4},
				{GLFW_KEY_F5, KeyCode::F5},
				{GLFW_KEY_F6, KeyCode::F6},
				{GLFW_KEY_F7, KeyCode::F7},
				{GLFW_KEY_F8, KeyCode::F8},
				{GLFW_KEY_F9, KeyCode::F9},
				{GLFW_KEY_F10, KeyCode::F10},
				{GLFW_KEY_F11, KeyCode::F11},
				{GLFW_KEY_F12, KeyCode::F12},
				{GLFW_KEY_KP_0, KeyCode::NumPadZero},
				{GLFW_KEY_KP_1, KeyCode::NumPadOne},
				{GLFW_KEY_KP_2, KeyCode::NumPadTwo},
				{GLFW_KEY_KP_3, KeyCode::NumPadThree},
				{GLFW_KEY_KP_4, KeyCode::NumPadFour},
				{GLFW_KEY_KP_5, KeyCode::NumPadFive},
				{GLFW_KEY_KP_6, KeyCode::NumPadSix},
				{GLFW_KEY_KP_7, KeyCode::NumPadSeven},
				{GLFW_KEY_KP_8, KeyCode::NumPadEight},
				{GLFW_KEY_KP_9, KeyCode::NumPadNine},
				{GLFW_KEY_KP_DECIMAL, KeyCode::NumPadDecimal},
				{GLFW_KEY_KP_DIVIDE, KeyCode::NumPadDivide},
				{GLFW_KEY_KP_MULTIPLY, KeyCode::NumPadMultiply},
				{GLFW_KEY_KP_SUBTRACT, KeyCode::NumPadSubtract},
				{GLFW_KEY_KP_ADD, KeyCode::NumPadAdd},
				{GLFW_KEY_KP_ENTER, KeyCode::NumPadEnter},
				{GLFW_KEY_KP_EQUAL, KeyCode::NumPadEqual},
				{GLFW_KEY_LEFT_SHIFT, KeyCode::LeftShift},
				{GLFW_KEY_LEFT_CONTROL, KeyCode::LeftControl},
				{GLFW_KEY_LEFT_ALT, KeyCode::LeftAlt},
				{GLFW_KEY_LEFT_SUPER, KeyCode::LeftSuper},
				{GLFW_KEY_RIGHT_SHIFT, KeyCode::RightShift},
				{GLFW_KEY_RIGHT_CONTROL, KeyCode::RightControl},
				{GLFW_KEY_RIGHT_ALT, KeyCode::RightAlt},
				{GLFW_KEY_RIGHT_SUPER, KeyCode::RightSuper},
				{GLFW_KEY_MENU, KeyCode::Menu},
				{GLFW_MOUSE_BUTTON_1, KeyCode::Mouse1},
				{GLFW_MOUSE_BUTTON_2, KeyCode::Mouse2},
				{GLFW_MOUSE_BUTTON_3, KeyCode::Mouse3},
				{GLFW_MOUSE_BUTTON_4, KeyCode::Mouse4},
				{GLFW_MOUSE_BUTTON_5, KeyCode::Mouse5},
				{GLFW_MOUSE_BUTTON_6, KeyCode::Mouse6},
				{GLFW_MOUSE_BUTTON_7, KeyCode::Mouse7},
				{GLFW_MOUSE_BUTTON_8, KeyCode::Mouse8}
			};

			KeyState keyState;

			switch (action)
			{
				case GLFW_PRESS:
				{
					keyState = KeyState::Down;
					break;
				}

				case GLFW_REPEAT:
				{
					keyState = KeyState::Held;
					break;
				}

				case GLFW_RELEASE:
				default:
				{
					keyState = KeyState::Up;
				}
			}

			auto const keyCode = keyCodes.at(key);
			EventManager::get_instance().send<KeyEvent>(keyCode, keyState);
		}
		catch (std::out_of_range const&)
		{
			Logger::get_instance().warning("Invalid key input detected.");
		}
	}



	void Window::mouse_callback(GLFWwindow*, double const x, double const y)
	{
		EventManager::get_instance().send<MouseEvent>(Vector2{x, y});
	}



	WindowEvent::WindowEvent(u32 const width, u32 const height, bool const fullscreen, bool const vsync) :
		width{width},
		height{height},
		fullscreen{fullscreen},
		vsync{vsync}
	{}
}
