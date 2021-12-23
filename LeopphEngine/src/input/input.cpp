#include "Input.hpp"

#include "../windowing/WindowBase.hpp"

namespace leopph
{
	std::map<KeyCode, KeyState> Input::s_KeyStates{};


	std::pair<float, float> Input::s_MousePos{};


	EventReceiverHandle<internal::KeyEvent> Input::keyEventReceiver{ [](const internal::KeyEvent& event)
	{
		s_KeyStates[event.keyCode] = event.keyState;
	} };


	EventReceiverHandle<internal::MouseEvent> Input::mouseEventReceiver{ [](const internal::MouseEvent& event)
	{
		s_MousePos = {event.position[0], event.position[1]};
	} };


	EventReceiverHandle<internal::FrameEndedEvent> Input::frameBeginsEventReceiver{ [](const internal::FrameEndedEvent&)
	{
		for (auto& [keyCode, keyState] : s_KeyStates)
		{
			if (keyState == KeyState::Up)
			{
				keyState = KeyState::Released;
			}
			else if (keyState == KeyState::Down)
			{
				keyState = KeyState::Held;
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
		return internal::WindowBase::Get().CursorMode();
	}


	void Input::CursorMode(CursorState newState)
	{
		internal::WindowBase::Get().CursorMode(newState);
	}
}