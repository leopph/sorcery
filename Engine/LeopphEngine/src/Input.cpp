#include "Input.hpp"

#include "InternalContext.hpp"
#include "windowing/WindowImpl.hpp"


namespace leopph
{
	bool Input::GetKey(KeyCode const key)
	{
		auto const state = InternalGetKeyState(key);
		return state == KeyState::Down || state == KeyState::Held;
	}


	bool Input::GetKeyDown(KeyCode const key)
	{
		return InternalGetKeyState(key) == KeyState::Down;
	}


	bool Input::GetKeyUp(KeyCode const key)
	{
		return InternalGetKeyState(key) == KeyState::Up;
	}


	std::pair<float, float> const& Input::GetMousePosition()
	{
		return s_MousePos;
	}


	CursorState Input::CursorMode()
	{
		return internal::GetWindowImpl()->get_cursor_state();
	}


	void Input::CursorMode(CursorState const newState)
	{
		internal::GetWindowImpl()->set_cursor_state(newState);
	}


	KeyState Input::InternalGetKeyState(KeyCode const keyCode) noexcept
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
