#pragma once

#include "CursorState.hpp"
#include "LeopphApi.hpp"

#include <string>
#include <string_view>
#include <vector>


namespace leopph
{
	class Window
	{
		public:
			// Get the width of the window.
			[[nodiscard]] virtual unsigned Width() const = 0;

			// Set the width of the window.
			virtual void Width(unsigned newWidth) = 0;

			// Get the height of the window.
			[[nodiscard]] virtual unsigned Height() const = 0;

			// Set the height of the window.
			virtual void Height(unsigned newHeight) = 0;

			// Get the aspect ratio (w/h) of the window.
			[[nodiscard]] LEOPPHAPI float AspectRatio() const;

			// Get whether the window has exclusive access to the display.
			[[nodiscard]] virtual bool Fullscreen() const = 0;

			// Set whether the window has exclusive access to the display.
			virtual void Fullscreen(bool newValue) = 0;

			// Get whether the window is vertically synced.
			[[nodiscard]] virtual bool Vsync() const = 0;

			// Set whether the window should be vertically synced.
			virtual void Vsync(bool newValue) = 0;

			// Get the title of the window.
			[[nodiscard]] virtual std::string_view Title() const = 0;

			// Set the title of the window.
			virtual void Title(std::string newTitle) = 0;

			// Get the current cursor state of the window.
			[[nodiscard]] virtual CursorState CursorMode() const = 0;

			// Set the cursor state of the window.
			virtual void CursorMode(CursorState newState) = 0;

			// Get the current render multiplier of the window.
			// Rendering operations use the resolution {width, height} * mult.
			[[nodiscard]] virtual float RenderMultiplier() const = 0;

			// Set the render multipler of the window.
			// Rendering operations will use the resolution {width, height} * mult.
			virtual void RenderMultiplier(float newMult) = 0;


			// Represents an acceptable configuration for a display.
			struct DisplayMode
			{
				unsigned Width;
				unsigned Height;
				unsigned RefreshRate;
			};


			// Returns all display modes the current or the primary monitor supports.
			// They are sorted in descending order.
			[[nodiscard]] virtual std::vector<DisplayMode> GetSupportedDisplayModes() const = 0;

			Window(Window const& other) = delete;
			void operator=(Window const& other) = delete;

			Window(Window&& other) = delete;
			void operator=(Window&& other) = delete;

		protected:
			Window() = default;
			virtual ~Window() = default;
	};
}
