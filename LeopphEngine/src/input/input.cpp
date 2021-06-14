#include "input.h"
#include "../windowing/window.h"
#include "../windowing/glwindow.h"

namespace leopph
{
	std::map<KeyCode, KeyState> Input::s_KeyStates{};
	std::pair<float, float> Input::s_MousePos{};


	
	void Input::OnInputChange(KeyCode keyCode, KeyState keyState)
	{
		s_KeyStates[keyCode] = keyState;
	}

	void Input::OnInputChange(double x, double y)
	{
		s_MousePos = { static_cast<float>(x), static_cast<float>(y) };
	}

	void Input::UpdateReleasedKeys()
	{
		for (auto& keyPair : s_KeyStates)
			if (keyPair.second == KeyState::Up)
				keyPair.second = KeyState::Released;
	}


	
	bool Input::GetKey(KeyCode key)
	{
		const KeyState state = s_KeyStates.at(key);
		return
			state == KeyState::Down ||
			state == KeyState::Held;
	}

	bool Input::GetKeyDown(KeyCode key)
	{
		return s_KeyStates.at(key) == KeyState::Down;
	}

	bool Input::GetKeyUp(KeyCode key)
	{
		return s_KeyStates.at(key) == KeyState::Up;
	}

	const std::pair<float, float>& Input::GetMousePosition()
	{
		return s_MousePos;
	}

	CursorState Input::CursorMode()
	{
		return impl::Window::Get().CursorMode();
	}

	void Input::CursorMode(CursorState newState)
	{
		impl::Window::Get().CursorMode(newState);
	}
}