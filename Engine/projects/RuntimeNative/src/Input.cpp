#include "Input.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cassert>


namespace leopph
{
	namespace
	{
		enum class Key : u8
		{
			Neutral = 0, Down = 1, Held = 2, Up = 3
		};

		Key gKeyboardState[256]{};
	}


	void update_keyboard_state()
	{
		BYTE newState[256];
		auto const success = GetKeyboardState(newState);
		assert(success);

		for (int i = 0; i < 256; i++)
		{
			if (newState[i] & 0x80)
			{
				if (gKeyboardState[i] == Key::Down)
				{
					gKeyboardState[i] = Key::Held;
				}
				else if (gKeyboardState[i] != Key::Held)
				{
					gKeyboardState[i] = Key::Down;
				}
			}
			else
			{
				if (gKeyboardState[i] == Key::Up)
				{
					gKeyboardState[i] = Key::Neutral;
				}
				else
				{
					gKeyboardState[i] = Key::Up;
				}
			}
		}
	}


	namespace detail
	{
		bool get_key(u8 const key)
		{
			return gKeyboardState[key] == Key::Down || gKeyboardState[key] == Key::Held;
		}


		bool get_key_down(u8 const key)
		{
			return gKeyboardState[key] == Key::Down;
		}


		bool get_key_up(u8 const key)
		{
			return gKeyboardState[key] == Key::Up;
		}
	}
}