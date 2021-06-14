#include "inputhandler.h"
#include "input.h"

namespace leopph::impl
{
	void InputHandler::OnInputChange(KeyCode keyCode, KeyState keyState)
	{
		Input::OnInputChange(keyCode, keyState);
	}

	void InputHandler::OnInputChange(double x, double y)
	{
		Input::OnInputChange(x, y);
	}


	void InputHandler::UpdateReleasedKeys()
	{
		Input::UpdateReleasedKeys();
	}

}