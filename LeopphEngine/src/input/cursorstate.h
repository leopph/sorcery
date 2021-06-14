#pragma once

/*-----------------------------------------------------------------------------------
List of all supported cursor states.
- The cursor is "Enabled", if it is visible to the user.
- The cursor is "Hidden", if it can be used to interact with the application,
  but is invisible to the user.
- The cursor is "Disabled", if it cannot be used to interact with the application.
  In this case, the mouse is constrained by the window and is allowed to freely roam.
-----------------------------------------------------------------------------------*/

enum class CursorState
{
	Shown,
	Hidden,
	Disabled
};