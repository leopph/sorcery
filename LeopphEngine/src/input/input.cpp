#include "Input.hpp"

#include "../windowing/Window.hpp"

#include <stdexcept>


namespace leopph
{
	auto Input::GetKey(const KeyCode key) -> bool
	{
		const auto state = InternalGetKeyState(key);
		return state == KeyState::Down || state == KeyState::Held;
	}


	auto Input::GetKeyDown(const KeyCode key) -> bool
	{
		return InternalGetKeyState(key) == KeyState::Down;
	}


	auto Input::GetKeyUp(const KeyCode key) -> bool
	{
		return InternalGetKeyState(key) == KeyState::Up;
	}


	auto Input::GetMousePosition() -> const std::pair<float, float>&
	{
		return s_MousePos;
	}


	auto Input::CursorMode() -> CursorState
	{
		return Window::Instance()->CursorMode();
	}


	auto Input::CursorMode(const CursorState newState) -> void
	{
		Window::Instance()->CursorMode(newState);
	}


	auto Input::InternalGetKeyState(const KeyCode keyCode) noexcept -> KeyState
	{
		try
		{
			return s_KeyStates.at(keyCode);
		}
		catch (std::out_of_range&)
		{
			return s_KeyStates[keyCode] = KeyState::Released;
		}
	}


	std::unordered_map<KeyCode, KeyState> Input::s_KeyStates;
	std::pair<float, float> Input::s_MousePos;

	EventReceiverHandle<internal::KeyEvent> Input::s_KeyEventReceiver{
		[](const internal::KeyEvent& event)
		{
			s_KeyStates[event.keyCode] = event.keyState;
		}
	};

	EventReceiverHandle<internal::MouseEvent> Input::s_MouseEventReceiver{
		[](const internal::MouseEvent& event)
		{
			s_MousePos = {event.position[0], event.position[1]};
		}
	};

	EventReceiverHandle<internal::FrameEndedEvent> Input::s_FrameBeginsEventReceiver{
		[](const internal::FrameEndedEvent&)
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
		}
	};
}
