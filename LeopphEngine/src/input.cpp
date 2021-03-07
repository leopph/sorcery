#include "input.h"
#include <GLFW/glfw3.h>
#include "window.h"

namespace leopph
{
	// list of keys
	const std::map<KeyCode, int> Input::s_KeyCodes
	{
		{KeyCode::ZERO, GLFW_KEY_0},
		{KeyCode::ONE, GLFW_KEY_1},
		{KeyCode::TWO, GLFW_KEY_2},
		{KeyCode::THREE, GLFW_KEY_3},
		{KeyCode::FOUR, GLFW_KEY_4},
		{KeyCode::FIVE, GLFW_KEY_5},
		{KeyCode::SIX, GLFW_KEY_6},
		{KeyCode::SEVEN, GLFW_KEY_7},
		{KeyCode::EIGHT, GLFW_KEY_8},
		{KeyCode::NINE, GLFW_KEY_9},

		{KeyCode::Q, GLFW_KEY_Q},
		{KeyCode::W, GLFW_KEY_W},
		{KeyCode::E, GLFW_KEY_E},
		{KeyCode::R, GLFW_KEY_R},
		{KeyCode::T, GLFW_KEY_T},
		{KeyCode::Y, GLFW_KEY_Y},
		{KeyCode::U, GLFW_KEY_U},
		{KeyCode::I, GLFW_KEY_I},
		{KeyCode::O, GLFW_KEY_O},
		{KeyCode::P, GLFW_KEY_P},
		{KeyCode::A, GLFW_KEY_A},
		{KeyCode::S, GLFW_KEY_S},
		{KeyCode::D, GLFW_KEY_D},
		{KeyCode::F, GLFW_KEY_F},
		{KeyCode::G, GLFW_KEY_G},
		{KeyCode::H, GLFW_KEY_H},
		{KeyCode::J, GLFW_KEY_J},
		{KeyCode::K, GLFW_KEY_K},
		{KeyCode::L, GLFW_KEY_L},
		{KeyCode::Z, GLFW_KEY_Z},
		{KeyCode::X, GLFW_KEY_X},
		{KeyCode::C, GLFW_KEY_C},
		{KeyCode::V, GLFW_KEY_V},
		{KeyCode::B, GLFW_KEY_B},
		{KeyCode::N, GLFW_KEY_N},
		{KeyCode::M, GLFW_KEY_M}
	};


	// init key states
	std::map<int, PressState> Input::s_KeyStates{};



	// init mouse pos
	std::pair<float, float> Input::s_MousePos{};
	


	// save keystate changes
	void Input::KeyCallback(int key, int action)
	{
		if (action == GLFW_PRESS)
			s_KeyStates[key] = PressState::Down;

		if (action == GLFW_REPEAT)
			s_KeyStates[key] = PressState::Held;

		if (action == GLFW_RELEASE)
			s_KeyStates[key] = PressState::Up;
	}


	void Input::MouseCallback(float x, float y)
	{
		s_MousePos.first = x;
		s_MousePos.second = y;
	}


	// register for callback
	void Input::RegisterCallbacks()
	{
		implementation::Window::SetKeyCallback(KeyCallback);
		implementation::Window::SetMouseCallback(MouseCallback);

		for (const auto& pair : s_KeyCodes)
			s_KeyStates[pair.second] = PressState::Released;
	}


	// change up states to released
	void Input::UpdateReleasedKeys()
	{
		for (auto& keyPair : s_KeyStates)
			if (keyPair.second == PressState::Up)
				keyPair.second = PressState::Released;
	}


	// keystate getters
	bool Input::GetKey(KeyCode key)
	{
		PressState state = s_KeyStates[s_KeyCodes.at(key)];
		return
			state == PressState::Down ||
			state == PressState::Held;
	}

	bool Input::GetKeyDown(KeyCode key)
	{
		return s_KeyStates[s_KeyCodes.at(key)] == PressState::Down;
	}

	bool Input::GetKeyUp(KeyCode key)
	{
		return s_KeyStates[s_KeyCodes.at(key)] == PressState::Up;
	}



	const std::pair<float, float>& Input::GetMousePosition()
	{
		return s_MousePos;
	}


	CursorState Input::CursorMode()
	{
		return implementation::Window::Get().CursorMode();
	}

	void Input::CursorMode(CursorState newState)
	{
		implementation::Window::Get().CursorMode(newState);
	}
}