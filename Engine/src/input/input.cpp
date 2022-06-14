#include "Input.hpp"

#include "../windowing/Window.hpp"


namespace leopph
{
	auto Input::GetKey(KeyCode const key) -> bool
	{
		auto const state = InternalGetKeyState(key);
		return state == KeyState::Down || state == KeyState::Held;
	}


	auto Input::GetKeyDown(KeyCode const key) -> bool
	{
		return InternalGetKeyState(key) == KeyState::Down;
	}


	auto Input::GetKeyUp(KeyCode const key) -> bool
	{
		return InternalGetKeyState(key) == KeyState::Up;
	}


	auto Input::GetMousePosition() -> std::pair<float, float> const&
	{
		return s_MousePos;
	}


	auto Input::CursorMode() -> CursorState
	{
		return Window::Instance()->CursorMode();
	}


	auto Input::CursorMode(CursorState const newState) -> void
	{
		Window::Instance()->CursorMode(newState);
	}


	auto Input::InternalGetKeyState(KeyCode const keyCode) noexcept -> KeyState
	{
		// Here we use the fact that KeyState default initializes to 0, which is "Released".
		// If we have no record of the key, we assume its released and zero-init the KeyState in the map.
		static_assert(std::is_same_v<decltype(s_KeyStates)::mapped_type, KeyState> && KeyState{} == KeyState::Released);
		return s_KeyStates[keyCode];
	}


	std::unordered_map<KeyCode, KeyState> Input::s_KeyStates;
	std::pair<float, float> Input::s_MousePos;

	EventReceiverHandle<internal::KeyEvent> Input::s_KeyEventReceiver{
		[](internal::KeyEvent const& event)
		{
			s_KeyStates[event.KeyCode] = event.KeyState;
		}
	};

	EventReceiverHandle<internal::MouseEvent> Input::s_MouseEventReceiver{
		[](internal::MouseEvent const& event)
		{
			s_MousePos = {event.Position[0], event.Position[1]};
		}
	};

	EventReceiverHandle<internal::FrameCompleteEvent> Input::s_FrameBeginsEventReceiver{
		[](internal::FrameCompleteEvent const&)
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
