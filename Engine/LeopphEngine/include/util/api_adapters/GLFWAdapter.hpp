#pragma once

#include "CursorState.hpp"
#include "KeyCode.hpp"
#include "KeyState.hpp"


namespace leopph::internal::glfw
{
	// Returns the abstract cursorstate mapped to the glfw cursor state.
	// Throws std::out_of_range if not found.
	[[nodiscard]] CursorState GetAbstractCursorState(int glfwCursorState);

	// Returns the glfw cursor state mapped to the abstract cursorstate.
	// Throws std::out_of_range if not found.
	[[nodiscard]] int GetGlfwCursorState(CursorState abstractCursorState);

	// Returns the abstract keycode mapped to the glfw key.
	// Throws std::out_of_range if not found.
	[[nodiscard]] KeyCode GetAbstractKeyCode(int glfwKey);

	// Returns the abstract keystate mapped to the glfw key state.
	// Throws std::out_of_range if not found.
	[[nodiscard]] KeyState GetAbstractKeyState(int glfwKeyState);
}
