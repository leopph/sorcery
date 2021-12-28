#include "Input.hpp"

#include "../windowing/WindowBase.hpp"


namespace leopph
{
	std::map<KeyCode, KeyState> Input::s_KeyStates;
	std::pair<float, float> Input::s_MousePos;

	EventReceiverHandle<internal::KeyEvent> Input::keyEventReceiver{
		[](const internal::KeyEvent& event)
		{
			s_KeyStates[event.keyCode] = event.keyState;
		}
	};

	EventReceiverHandle<internal::MouseEvent> Input::mouseEventReceiver{
		[](const internal::MouseEvent& event)
		{
			s_MousePos = {event.position[0], event.position[1]};
		}
	};

	EventReceiverHandle<internal::FrameEndedEvent> Input::frameBeginsEventReceiver{
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


	auto Input::GetKey(const KeyCode key) -> bool
	{
		const auto state = s_KeyStates.at(key);
		return state == KeyState::Down || state == KeyState::Held;
	}


	auto Input::GetKeyDown(const KeyCode key) -> bool
	{
		return s_KeyStates.at(key) == KeyState::Down;
	}


	auto Input::GetKeyUp(const KeyCode key) -> bool
	{
		return s_KeyStates.at(key) == KeyState::Up;
	}


	auto Input::GetMousePosition() -> const std::pair<float, float>&
	{
		return s_MousePos;
	}


	auto Input::CursorMode() -> CursorState
	{
		return internal::WindowBase::Get().CursorMode();
	}


	auto Input::CursorMode(const CursorState newState) -> void
	{
		internal::WindowBase::Get().CursorMode(newState);
	}
}
