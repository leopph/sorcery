#pragma once

#include "LeopphApi.hpp"
#include "CursorState.hpp"

#include <string>
#include <string_view>
#include <vector>


namespace leopph
{
	class Window
	{
		public:
			// Get the width of the window.
			[[nodiscard]] virtual auto Width() const -> unsigned = 0;

			// Set the width of the window.
			virtual auto Width(unsigned newWidth) -> void = 0;

			// Get the height of the window.
			[[nodiscard]] virtual auto Height() const -> unsigned = 0;

			// Set the height of the window.
			virtual auto Height(unsigned newHeight) -> void = 0;

			// Get the aspect ratio (w/h) of the window.
			[[nodiscard]] auto LEOPPHAPI AspectRatio() const -> float;

			// Get whether the window has exclusive access to the display.
			[[nodiscard]] virtual auto Fullscreen() const -> bool = 0;

			// Set whether the window has exclusive access to the display.
			auto virtual Fullscreen(bool newValue) -> void = 0;

			// Get whether the window is vertically synced.
			[[nodiscard]] virtual auto Vsync() const -> bool = 0;

			// Set whether the window should be vertically synced.
			virtual auto  Vsync(bool newValue) -> void = 0;

			// Get the title of the window.
			[[nodiscard]] virtual auto  Title() const -> std::string_view = 0;

			// Set the title of the window.
			auto virtual Title(std::string newTitle) -> void = 0;

			// Get the current cursor state of the window.
			[[nodiscard]] virtual auto CursorMode() const -> CursorState = 0;

			// Set the cursor state of the window.
			virtual auto CursorMode(CursorState newState) -> void = 0;

			// Get the current render multiplier of the window.
			// Rendering operations use the resolution {width, height} * mult.
			[[nodiscard]] virtual auto RenderMultiplier() const -> float = 0;

			// Set the render multipler of the window.
			// Rendering operations will use the resolution {width, height} * mult.
			virtual auto RenderMultiplier(float newMult) -> void = 0;


			// Represents an acceptable configuration for a display.
			struct DisplayMode
			{
				unsigned Width;
				unsigned Height;
				unsigned RefreshRate;
			};


			// Returns all display modes the current or the primary monitor supports.
			// They are sorted in descending order.
			[[nodiscard]] virtual auto GetSupportedDisplayModes() const -> std::vector<DisplayMode> = 0;

			Window(const Window& other) = delete;
			auto operator=(const Window& other) -> void = delete;

			Window(Window&& other) = delete;
			auto operator=(Window&& other) -> void = delete;

		protected:
			Window() = default;
			virtual ~Window() = default;
	};
}
