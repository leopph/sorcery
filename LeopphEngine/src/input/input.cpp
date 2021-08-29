#include "Input.hpp"

#include "../windowing/window.h"

namespace leopph
{
	std::map<KeyCode, KeyState> Input::s_KeyStates{};


	std::pair<float, float> Input::s_MousePos{};


	EventReceiverHandle<impl::KeyEvent> Input::keyEventReceiver{ [](const impl::KeyEvent& event)
	{
		s_KeyStates[event.keyCode] = event.keyState;
	} };


	EventReceiverHandle<impl::MouseEvent> Input::mouseEventReceiver{ [](const impl::MouseEvent& event)
	{
		s_MousePos = {event.position[0], event.position[1]};
	} };


	EventReceiverHandle<impl::FrameBeginsEvent> Input::frameBeginsEventReceiver{ [](const impl::FrameBeginsEvent& event)
	{
		for (auto& [keyCode, keyState] : s_KeyStates)
		{
			if (keyState == KeyState::Up)
			{
				keyState = KeyState::Released;
			}
		}
	} };

	
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