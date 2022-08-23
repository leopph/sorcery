#include "Input.hpp"

#include <ranges>


namespace leopph
{
	KeyEvent::KeyEvent(leopph::KeyCode const keyCode, leopph::KeyState const keyState) :
		keyCode{keyCode},
		keyState{keyState}
	{}



	MouseEvent::MouseEvent(Vector2 const pos) :
		position{pos}
	{}



	bool Input::get_key(KeyCode const key)
	{
		auto const state = s_KeyStates[key];
		return state == KeyState::Down || state == KeyState::Held;
	}



	bool Input::get_key_down(KeyCode const key)
	{
		return s_KeyStates[key] == KeyState::Down;
	}



	bool Input::get_key_up(KeyCode const key)
	{
		return s_KeyStates[key] == KeyState::Up;
	}



	Vector2 const& Input::get_mouse_position()
	{
		return s_MousePos;
	}



	EventReceiverHandle<KeyEvent> Input::s_KeyEventReceiver
	{
		[](KeyEvent const& event)
		{
			s_KeyStates[event.keyCode] = event.keyState;
		}
	};

	EventReceiverHandle<MouseEvent> Input::s_MouseEventReceiver
	{
		[](MouseEvent const& event)
		{
			s_MousePos = {event.position[0], event.position[1]};
		}
	};

	EventReceiverHandle<internal::FrameCompleteEvent> Input::s_FrameBeginsEventReceiver
	{
		[](internal::FrameCompleteEvent const&)
		{
			for (auto& keyState : s_KeyStates | std::views::values)
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


	std::unordered_map<KeyCode, KeyState> Input::s_KeyStates;
	Vector2 Input::s_MousePos;
}
