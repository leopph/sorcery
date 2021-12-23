#pragma once

#include "../input/keycodes.h"
#include "../input/keystate.h"
#include "Event.hpp"


namespace leopph::internal
{
	struct KeyEvent final : Event
	{
		const KeyCode keyCode;
		const KeyState keyState;

		KeyEvent(KeyCode keyCode, KeyState keyState);
	};
}