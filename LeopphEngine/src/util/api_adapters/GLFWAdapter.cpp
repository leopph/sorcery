#include "GLFWAdapter.hpp"

#include "../containers/Bimap.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <functional>
#include <unordered_map>


namespace leopph::internal::glfw
{
	static Bimap<int,
	             CursorState,
	             decltype([](const auto elem)
	             {
		             return std::hash<int>{}(static_cast<int>(elem));
	             }),
	             decltype([](const auto left, const auto right)
	             {
		             return left == right;
	             }),
	             #ifdef _DEBUG
	             true>
	             #else
	             false>
	#endif
	s_CursorStates
	{
		{GLFW_CURSOR_NORMAL, CursorState::Shown},
		{GLFW_CURSOR_HIDDEN, CursorState::Hidden},
		{GLFW_CURSOR_DISABLED, CursorState::Disabled}
	};

	static std::unordered_map<int, KeyCode> s_KeyCodes
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

	const std::unordered_map<int, KeyState> s_KeyStates
	{
		{GLFW_PRESS, KeyState::Down},
		{GLFW_REPEAT, KeyState::Held},
		{GLFW_RELEASE, KeyState::Up}
	};


	auto GetAbstractCursorState(const int glfwCursorState) -> CursorState
	{
		return s_CursorStates.At(glfwCursorState);
	}


	auto GetGlfwCursorState(const CursorState abstractCursorState) -> int
	{
		return s_CursorStates.At(abstractCursorState);
	}


	auto GetAbstractKeyCode(const int glfwKey) -> KeyCode
	{
		return s_KeyCodes.at(glfwKey);
	}


	auto GetAbstractKeyState(const int glfwKeyState) -> KeyState
	{
		return s_KeyStates.at(glfwKeyState);
	}
}
