#pragma once

// All supported cursor states.
enum class CursorState
{
	// The cursor is Enabled, if it is visible to the user.
	Shown,
	// The cursor is Hidden, if it can be used to interact with the application, but is invisible to the user.
	Hidden,
	// The cursor is Disabled, if it cannot be used to interact with the application.
	// In this case, mouse movements are unconstrained.
	Disabled
};
