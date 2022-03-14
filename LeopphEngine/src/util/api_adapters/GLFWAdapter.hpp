#pragma once

#include "../../input/KeyCode.hpp"
#include "../../input/KeyState.hpp"


namespace leopph::internal::glfw
{
	// Returns the abstract keycode mapped to the glfw key.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetAbstractKeyCode(int glfwKey) -> KeyCode;

	// Returns the abstract keystate mapped to the glfw key state.
	// Throws std::out_of_range if not found.
	[[nodiscard]] auto GetAbstractKeyState(int glfwKeyState) -> KeyState;
}
