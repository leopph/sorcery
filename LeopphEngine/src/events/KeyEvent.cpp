#include "KeyEvent.hpp"


namespace leopph::impl
{
	KeyEvent::KeyEvent(const KeyCode keyCode, const KeyState keyState) :
		keyCode{ keyCode }, keyState{ keyState }
	{}
}