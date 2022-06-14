#pragma once

#include "Event.hpp"
#include "../input/KeyCode.hpp"
#include "../input/KeyState.hpp"


namespace leopph::internal
{
	// An event representing a key status change.
	struct KeyEvent final : Event
	{
		KeyEvent(KeyCode keyCode, KeyState keyState);
		KeyCode KeyCode;
		KeyState KeyState;
	};
}
