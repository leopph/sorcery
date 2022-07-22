#include "KeyEvent.hpp"


namespace leopph::internal
{
	KeyEvent::KeyEvent(leopph::KeyCode const keyCode, leopph::KeyState const keyState) :
		KeyCode{keyCode},
		KeyState{keyState}
	{}
}
