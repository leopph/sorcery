#pragma once

#include <map>
#include <GLFW/glfw3.h>

#include "leopphapi.h"


namespace leopph
{
	enum class KeyCode
	{
		ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
		Q, W, E, R, T, Y, U, I, O, P,
		A, S, D, F, G, H, J, K, L,
		Z, X, C, V, B, N, M
	};


	enum class PressState
	{
		Down, Held, Up, Released,
	};


#pragma warning (push)
#pragma warning (disable: 4251)

	class LEOPPHAPI Input
	{
	private:
		const static std::map<KeyCode, int> s_KeyCodes;
		static std::map<int, PressState> s_KeyStates;

		static void KeyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

	public:
		static void RegisterWindow(GLFWwindow* window);

		static void UpdateReleasedKeys();

		static bool GetKey(KeyCode key);
		static bool GetKeyDown(KeyCode key);
		static bool GetKeyUp(KeyCode key);
	};

#pragma warning (pop)
}