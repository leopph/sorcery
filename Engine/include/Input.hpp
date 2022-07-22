#pragma once

#include "CursorState.hpp"
#include "EventReceiverHandle.hpp"
#include "FrameCompleteEvent.hpp"
#include "KeyCode.hpp"
#include "KeyEvent.hpp"
#include "KeyState.hpp"
#include "LeopphApi.hpp"
#include "MouseEvent.hpp"

#include <unordered_map>
#include <utility>


namespace leopph
{
	// The Input class provides access to information related to device inputs.
	class Input
	{
		public:
			// Returns true if the passed key is being pressed down in the current frame.
			// Returns true for held keys.
			LEOPPHAPI static bool GetKey(KeyCode key);

			// Returns true if the passed key was pressed in the current frame. 
			// Returns false for held keys.
			LEOPPHAPI static bool GetKeyDown(KeyCode key);

			// Returns true if the passed key was released in this frame.
			// Returns false for untouched keys.
			LEOPPHAPI static bool GetKeyUp(KeyCode key);

			// Returns the (x, y) pair of mouse coordinates.
			LEOPPHAPI static const std::pair<float, float>& GetMousePosition();

			// Get whether the Cursor is Shown, Hidden, or Disabled.
			LEOPPHAPI static CursorState CursorMode();

			// Set whether the Cursor is Shown, Hidden, or Disabled.
			LEOPPHAPI static void CursorMode(CursorState newState);

		private:
			// Returns the keystate mapped to the keycode.
			// Inserts KeyState::Released if not found.
			// Use this to read key states.
			[[nodiscard]] static KeyState InternalGetKeyState(KeyCode keyCode) noexcept;

			static EventReceiverHandle<internal::KeyEvent> s_KeyEventReceiver;
			static EventReceiverHandle<internal::MouseEvent> s_MouseEventReceiver;
			static EventReceiverHandle<internal::FrameCompleteEvent> s_FrameBeginsEventReceiver;
			static std::unordered_map<KeyCode, KeyState> s_KeyStates;
			static std::pair<float, float> s_MousePos;
	};
}
