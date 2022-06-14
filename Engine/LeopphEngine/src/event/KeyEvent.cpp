#include "KeyEvent.hpp"


namespace leopph::internal
{
	KeyEvent::KeyEvent(const leopph::KeyCode keyCode, const leopph::KeyState keyState) :
		KeyCode{keyCode},
		KeyState{keyState}
	{}
}
