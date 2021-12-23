#include "GlWindow.hpp"

#include "../events/KeyEvent.hpp"
#include "../events/MouseEvent.hpp"
#include "../events/ScreenResolutionEvent.hpp"
#include "../events/handling/EventManager.hpp"
#include "../input/Input.hpp"
#include "../util/logger.h"

#include <glad/glad.h>

#include <array>
#include <map>
#include <stdexcept>
#include <utility>


namespace leopph::internal
{
	GlWindow::GlWindow(const int width, const int height, const std::string& title, const bool fullscreen) :
		WindowBase{},
		m_Width{width},
		m_Height{height},
		m_Fullscreen{fullscreen},
		m_Vsync{false},
		m_Title{title},
		m_Background{},
		m_RenderMult{1.f}
	{
		if (!glfwInit())
		{
			const auto erroMsg{"Failed to initialize GLFW."};
			Logger::Instance().Error(erroMsg);
			throw std::runtime_error{erroMsg};
		}

		int major, minor, rev;
		glfwGetVersion(&major, &minor, &rev);
		Logger::Instance().Debug("Using GLFW " + std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(rev) + ".");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		const auto monitor{m_Fullscreen ? glfwGetPrimaryMonitor() : nullptr};
		m_Window = glfwCreateWindow(width, height, title.data(), monitor, nullptr);

		if (m_Window == nullptr)
		{
			const auto errMsg{"Failed to create window."};
			Logger::Instance().Error(errMsg);
			throw std::runtime_error{errMsg};
		}

		glfwMakeContextCurrent(m_Window);
		glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
		glfwSetKeyCallback(m_Window, KeyCallback);
		glfwSetCursorPosCallback(m_Window, MouseCallback);
		glfwSetCursorPos(m_Window, 0, 0);
		glfwSwapInterval(0);
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

	auto GlWindow::Background() const -> const Color&
	{
		return m_Background;
	}

	auto GlWindow::Background(const Color& color) -> void
	{
		m_Background = color;
	}

	auto GlWindow::CursorMode() const -> CursorState
	{
		static const std::map<decltype(GLFW_CURSOR_NORMAL), CursorState> cursorStates
		{
			{GLFW_CURSOR_NORMAL, CursorState::Shown},
			{GLFW_CURSOR_HIDDEN, CursorState::Hidden},
			{GLFW_CURSOR_DISABLED, CursorState::Disabled}
		};

		return cursorStates.at(glfwGetInputMode(this->m_Window, GLFW_CURSOR));
	}

	auto GlWindow::CursorMode(const CursorState newState) -> void
	{
		static const std::map<CursorState, decltype(GLFW_CURSOR_NORMAL)> cursorStates
		{
			{CursorState::Shown, GLFW_CURSOR_NORMAL},
			{CursorState::Hidden, GLFW_CURSOR_HIDDEN},
			{CursorState::Disabled, GLFW_CURSOR_DISABLED}
		};

		glfwSetInputMode(this->m_Window, GLFW_CURSOR, cursorStates.at(newState));
	}

	auto GlWindow::RenderMultiplier() -> float
	{
		return m_RenderMult;
	}

	auto GlWindow::RenderMultiplier(const float newMult) -> void
	{
		m_RenderMult = newMult;
		EventManager::Instance().Send<ScreenResolutionEvent>(Vector2{m_Width, m_Height}, m_RenderMult);
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

	auto GlWindow::Clear() -> void
	{
		glClearNamedFramebufferfv(0, GL_COLOR, 0, Vector4{static_cast<Vector3>(Background())}.Data().data());
		glClearNamedFramebufferfv(0, GL_DEPTH, 0, std::array{1.f}.data());
	}

	auto GlWindow::InitKeys() -> void
	{
		for (const auto& [_, keyCode] : s_KeyCodes)
		{
			EventManager::Instance().Send<KeyEvent>(keyCode, KeyState::Released);
		}
	}

	auto GlWindow::FramebufferSizeCallback(GLFWwindow*, const int width, const int height) -> void
	{
		glViewport(0, 0, width, height);

		auto& windowInstance{static_cast<GlWindow&>(Get())};
		windowInstance.m_Width = width;
		windowInstance.m_Height = height;

		EventManager::Instance().Send<ScreenResolutionEvent>(Vector2{width, height}, windowInstance.RenderMultiplier());
	}

	auto GlWindow::KeyCallback(GLFWwindow*, const int key, int, const int action, int) -> void
	{
		try
		{
			const auto keyCode = s_KeyCodes.at(key);
			const auto keyState = s_KeyStates.at(action);
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

	const std::unordered_map<int, KeyState> GlWindow::s_KeyStates
	{
		{GLFW_PRESS, KeyState::Down},
		{GLFW_REPEAT, KeyState::Held},
		{GLFW_RELEASE, KeyState::Up}
	};

	const std::unordered_map<int, KeyCode> GlWindow::s_KeyCodes
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
}
