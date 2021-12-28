#pragma once

#include "CursorState.hpp"
#include "KeyCode.hpp"
#include "KeyState.hpp"
#include "../api/LeopphApi.hpp"
#include "../events/FrameEndEvent.hpp"
#include "../events/KeyEvent.hpp"
#include "../events/MouseEvent.hpp"
#include "../events/handling/EventReceiverHandle.hpp"

#include <map>
#include <utility>


namespace leopph
{
	// The Input class provides access to information related to device inputs.
	class Input
	{
		public:
			// Returns true if the passed key is being pressed down in the current frame.
			// Returns true for held keys.
			LEOPPHAPI static auto GetKey(KeyCode key) -> bool;

			// Returns true if the passed key was pressed in the current frame. 
			// Returns false for held keys.
			LEOPPHAPI static auto GetKeyDown(KeyCode key) -> bool;

			// Returns true if the passed key was released in this frame.
			// Returns false for untouched keys.
			LEOPPHAPI static auto GetKeyUp(KeyCode key) -> bool;

			// Returns the (x, y) pair of mouse coordinates.
			LEOPPHAPI static auto GetMousePosition() -> const std::pair<float, float>&;

			// Get whether the Cursor is Shown, Hidden, or Disabled.
			LEOPPHAPI static auto CursorMode() -> CursorState;

			// Set whether the Cursor is Shown, Hidden, or Disabled.
			LEOPPHAPI static auto CursorMode(CursorState newState) -> void;

		private:
			static std::map<KeyCode, KeyState> s_KeyStates;
			static std::pair<float, float> s_MousePos;

			static EventReceiverHandle<internal::KeyEvent> keyEventReceiver;
			static EventReceiverHandle<internal::MouseEvent> mouseEventReceiver;
			static EventReceiverHandle<internal::FrameEndedEvent> frameBeginsEventReceiver;
	};
}
