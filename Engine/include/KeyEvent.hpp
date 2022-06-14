#pragma once

#include "Event.hpp"
#include "KeyCode.hpp"
#include "KeyState.hpp"


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
