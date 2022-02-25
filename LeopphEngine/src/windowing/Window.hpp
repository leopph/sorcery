#pragma once

#include "../api/LeopphApi.hpp"
#include "../input/CursorState.hpp"

#include <string>
#include <string_view>


namespace leopph
{
	class Window
	{
		public:
			// Returns a pointer to the currently active window instance.
			// The instance has to be initialized by the engine first.
			// Otherwise this returns nullptr.
			[[nodiscard]] LEOPPHAPI static auto Instance() -> Window*;

			// Get the width of the window.
			[[nodiscard]] LEOPPHAPI virtual auto Width() const -> unsigned = 0;
			// Get the height of the window.
			[[nodiscard]] LEOPPHAPI virtual auto Height() const -> unsigned = 0;
			// Get the aspect ratio (w/h) of the window.
			[[nodiscard]] LEOPPHAPI auto AspectRatio() const -> float;
			// Get whether the window has exclusive access to the display.
			[[nodiscard]] LEOPPHAPI virtual auto Fullscreen() const -> bool = 0;
			// Get whether the window is vertically synced.
			[[nodiscard]] LEOPPHAPI virtual auto Vsync() const -> bool = 0;
			// Get the title of the window.
			[[nodiscard]] LEOPPHAPI virtual auto Title() const -> std::string_view = 0;
			// Get the current cursor state of the window.
			[[nodiscard]] LEOPPHAPI virtual auto CursorMode() const -> CursorState = 0;
			// Get the current render multiplier of the window.
			// Rendering operations use the resolution {width, height} * mult.
			[[nodiscard]] LEOPPHAPI virtual auto RenderMultiplier() const -> float = 0;

			// Set the width of the window.
			LEOPPHAPI virtual auto Width(unsigned newWidth) -> void = 0;
			// Set the height of the window.
			LEOPPHAPI virtual auto Height(unsigned newHeight) -> void = 0;
			// Set whether the window has exclusive access to the display.
			LEOPPHAPI virtual auto Fullscreen(bool newValue) -> void = 0;
			// Set whether the window should be vertically synced.
			LEOPPHAPI virtual auto Vsync(bool newValue) -> void = 0;
			// Set the title of the window.
			LEOPPHAPI virtual auto Title(std::string newTitle) -> void = 0;
			// Set the cursor state of the window.
			LEOPPHAPI virtual auto CursorMode(CursorState newState) -> void = 0;
			// Set the render multipler of the window.
			// Rendering operations will use the resolution {width, height} * mult.
			LEOPPHAPI virtual auto RenderMultiplier(float newMult) -> void = 0;

			Window(const Window& other) = delete;
			auto operator=(const Window& other) -> Window& = delete;

			Window(Window&& other) = delete;
			auto operator=(Window&& other) -> Window& = delete;

		protected:
			// Register itself as static instance
			Window();
			// Sets the static instance to nullptr
			virtual ~Window();

		private:
			static Window* s_Instance;
	};
}
