#pragma once

#include <map>
#include <utility>

#include "../api/leopphapi.h"
#include "../windowing/window.h"


namespace leopph
{
	/*---------------------------------------------
	A list of all supported keys.
	Use these to get information about user inputs.
	---------------------------------------------*/
	// TODO separate header

	enum class KeyCode
	{
		ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L,
		Z, X, C, V, B, N, M
	};


	/*---------------------------------------------
	A list of all supported keystates.
	Use these to get information about user inputs.
	---------------------------------------------*/
	// TODO separater header

	enum class PressState
	{
		Down, Held, Up, Released,
	};

	
	// TODO separate header
	using CursorState = impl::Window::CursorState;


	/*--------------------------------------------------------------------
	The Input class provides ways to gather information about user inputs.
	--------------------------------------------------------------------*/

	class Input
	{
	public:
		/* Internally used functions */
		// TODO these should not be accessible to clients
		LEOPPHAPI static void RegisterCallbacks();
		LEOPPHAPI static void UpdateReleasedKeys();

		/* Returns true if the given key is being pressed down in the current frame.
		Returns true for held keys. */
		LEOPPHAPI static bool GetKey(KeyCode key);

		/* Returns true if the given key was pressed in the current frame. 
		Returns false for held keys. */
		LEOPPHAPI static bool GetKeyDown(KeyCode key);

		/* Returns true if the given key was released in this frame.
		Returns false for untouched keys. */
		LEOPPHAPI static bool GetKeyUp(KeyCode key);

		/* Returns an (x, y) pair of mouse coordinates */
		LEOPPHAPI static const std::pair<float, float>& GetMousePosition();

		/* Determines whether the Cursor is Shown, Hidden, or Disabled */
		LEOPPHAPI static CursorState CursorMode();
		LEOPPHAPI static void CursorMode(CursorState newState);

	private:
		const static std::map<KeyCode, int> s_KeyCodes;
		static std::map<int, PressState> s_KeyStates;
		static std::pair<float, float> s_MousePos;

		static void KeyCallback(int key, int action);
		static void MouseCallback(float x, float y);
	};
}