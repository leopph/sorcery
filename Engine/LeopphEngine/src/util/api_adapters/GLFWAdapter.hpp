#pragma once

#include "../../input/CursorState.hpp"
#include "../../input/KeyCode.hpp"
#include "../../input/KeyState.hpp"


namespace leopph::internal::glfw
{
	// Returns the abstract cursorstate mapped to the glfw cursor state.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetAbstractCursorState(int glfwCursorState) -> CursorState;

	// Returns the glfw cursor state mapped to the abstract cursorstate.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetGlfwCursorState(CursorState abstractCursorState) -> int;

	// Returns the abstract keycode mapped to the glfw key.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetAbstractKeyCode(int glfwKey) -> KeyCode;

	// Returns the abstract keystate mapped to the glfw key state.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetAbstractKeyState(int glfwKeyState) -> KeyState;
}
