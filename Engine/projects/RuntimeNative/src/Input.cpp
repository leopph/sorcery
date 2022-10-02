#include "RuntimeNative.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cassert>

namespace
{
	unsigned char constexpr KEY_NEUTRAL = 0;
	unsigned char constexpr KEY_DOWN = 1;
	unsigned char constexpr KEY_HELD = 2;
	unsigned char constexpr KEY_UP = 3;

	unsigned char gKeyboardState[256]{};
}

extern "C"
{
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


	bool get_key(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN || gKeyboardState[key] == KEY_HELD;
	}


	bool get_key_down(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_DOWN;
	}


	bool get_key_up(unsigned char const key)
	{
		return gKeyboardState[key] == KEY_UP;
	}
}