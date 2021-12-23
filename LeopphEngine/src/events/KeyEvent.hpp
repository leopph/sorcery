#pragma once

#include "Event.hpp"
#include "../input/keycodes.h"
#include "../input/keystate.h"


namespace leopph::internal
{
	struct KeyEvent final : Event
	{
		const KeyCode keyCode;
		const KeyState keyState;

		KeyEvent(KeyCode keyCode, KeyState keyState);
	};
}
