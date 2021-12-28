#pragma once

#include "Event.hpp"
#include "../input/KeyCode.hpp"
#include "../input/KeyState.hpp"


namespace leopph::internal
{
	struct KeyEvent final : Event
	{
		const KeyCode keyCode;
		const KeyState keyState;

		KeyEvent(KeyCode keyCode, KeyState keyState);
	};
}
