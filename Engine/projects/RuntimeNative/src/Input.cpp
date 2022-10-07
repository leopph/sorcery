#include "Input.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cassert>

namespace leopph
{
	namespace
	{
		enum Key : u8
		{
			KEY_NEUTRAL = 0,
			KEY_DOWN = 1,
			KEY_HELD = 2,
			KEY_UP = 3,
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
				if (gKeyboardState[i] == KEY_DOWN)
				{
					gKeyboardState[i] = KEY_HELD;
				}
				else
				{
					gKeyboardState[i] = KEY_DOWN;
				}
			}
			else
			{
				if (gKeyboardState[i] == KEY_UP)
				{
					gKeyboardState[i] = KEY_NEUTRAL;
				}
				else
				{
					gKeyboardState[i] = KEY_UP;
				}
			}
		}
	}


	extern "C"
	{
		__declspec(dllexport) bool get_key(u8 const key)
		{
			return gKeyboardState[key] == KEY_DOWN || gKeyboardState[key] == KEY_HELD;
		}


		__declspec(dllexport) bool get_key_down(u8 const key)
		{
			return gKeyboardState[key] == KEY_DOWN;
		}


		__declspec(dllexport) bool get_key_up(u8 const key)
		{
			return gKeyboardState[key] == KEY_UP;
		}
	}
}