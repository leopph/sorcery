#pragma once

#include <map>

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

		static void KeyCallback(int key, int action);

	public:
		static void RegisterCallback(); // TODO abstract glfw away

		static void UpdateReleasedKeys();

		static bool GetKey(KeyCode key);
		static bool GetKeyDown(KeyCode key);
		static bool GetKeyUp(KeyCode key);
	};

#pragma warning (pop)
}