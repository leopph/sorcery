#pragma once

namespace leopph
{
	// All supported key states.
	enum class KeyState : int
	{
		Down = 1, // The key was pressed down during this frame.
		Held = 2, // The key has been pressed down since an earlier frame.
		Up = 3, // The key was let go during this frame.
		Released = 0, // The key is in its natural state.
	};
}
