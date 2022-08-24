#pragma once

#include "Event.hpp"
#include "Event.hpp"
#include "FrameCompleteEvent.hpp"
#include "LeopphApi.hpp"
#include "Math.hpp"
#include "Types.hpp"

#include <unordered_map>


namespace leopph
{
	enum class KeyCode : i16;
	enum class KeyState : i8;


	struct KeyEvent final : Event
	{
		KeyEvent(KeyCode keyCode, KeyState keyState);

		KeyCode keyCode;
		KeyState keyState;
	};


	struct MouseEvent final : Event
	{
		explicit MouseEvent(Vector2 pos);

		Vector2 position;
	};


	// The Input class provides access to information related to device inputs.
	class Input
	{
		public:
			// Returns true if the passed key is being pressed down in the current frame.
			// Returns true for held keys.
			LEOPPHAPI static bool get_key(KeyCode key);

			// Returns true if the passed key was pressed in the current frame. 
			// Returns false for held keys.
			LEOPPHAPI static bool get_key_down(KeyCode key);

			// Returns true if the passed key was released in this frame.
			// Returns false for untouched keys.
			LEOPPHAPI static bool get_key_up(KeyCode key);

			// Returns the (x, y) pair of mouse coordinates.
			LEOPPHAPI static Vector2 const& get_mouse_position();

		private:
			static EventReceiverHandle<KeyEvent> s_KeyEventReceiver;
			static EventReceiverHandle<MouseEvent> s_MouseEventReceiver;
			static EventReceiverHandle<internal::FrameCompleteEvent> s_FrameBeginsEventReceiver;
			static std::unordered_map<KeyCode, KeyState> s_KeyStates;
			static Vector2 s_MousePos;
	};


	enum class KeyCode : i16
	{
		Zero,
		One,
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Space,
		Apostrophe,
		Comma,
		Minus,
		Period,
		Slash,
		Semicolon,
		Equal,
		LeftBracket,
		Backslash,
		RightBracket,
		GraveAccent,
		Escape,
		Enter,
		Tab,
		Backspace,
		Insert,
		Delete,
		Right,
		Left,
		Down,
		Up,
		PageUp,
		PageDown,
		Home,
		End,
		CapsLock,
		ScrollLock,
		NumLock,
		PrintScreen,
		Pause,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		NumPadZero,
		NumPadOne,
		NumPadTwo,
		NumPadThree,
		NumPadFour,
		NumPadFive,
		NumPadSix,
		NumPadSeven,
		NumPadEight,
		NumPadNine,
		NumPadDecimal,
		NumPadDivide,
		NumPadMultiply,
		NumPadSubtract,
		NumPadAdd,
		NumPadEnter,
		NumPadEqual,
		LeftShift,
		LeftControl,
		LeftAlt,
		LeftSuper,
		RightShift,
		RightControl,
		RightAlt,
		RightSuper,
		Menu,
		Mouse1,
		Mouse2,
		Mouse3,
		Mouse4,
		Mouse5,
		Mouse6,
		Mouse7,
		Mouse8
	};


	enum class KeyState : i8
	{
		Down = 1, // The key was pressed down during this frame.
		Held = 2, // The key has been pressed down since an earlier frame.
		Up = 3, // The key was let go during this frame.
		Released = 0, // The key is in its natural state.
	};


	static_assert(KeyState{} == KeyState::Released);
}
