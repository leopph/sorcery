#pragma once

namespace leopph
{
	// All supported key states.
	enum class KeyState
	{
		// A key is Down if it was pressed down during this frame.
		Down,
		// A key is Held if it has been pressed down since an earlier frame.
		Held,
		// A key is Up if it was let go during this frame.
		Up,
		// A key is Released in its natural state.
		Released,
	};
}
