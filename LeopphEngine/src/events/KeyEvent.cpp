#include "KeyEvent.hpp"


namespace leopph::internal
{
	KeyEvent::KeyEvent(const KeyCode keyCode, const KeyState keyState) :
		keyCode{ keyCode }, keyState{ keyState }
	{}
}