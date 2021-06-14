#pragma once

#include "keycodes.h"
#include "keystate.h"
#include "cursorstate.h"

namespace leopph::impl
{	
	/*---------------------------------------------------------
	Middle layer between Window implementations and Input class
	to decouple implementation details.
	---------------------------------------------------------*/
	
	class InputHandler
	{
	public:
		static void OnInputChange(KeyCode keyCode, KeyState keyState);
		static void OnInputChange(double x, double y);
	};
}