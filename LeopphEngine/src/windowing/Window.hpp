#pragma once

#include "../api/LeopphApi.hpp"

#include <string>
#include <string_view>


namespace leopph
{
	class Window
	{
		public:
			[[nodiscard]]
			LEOPPHAPI static unsigned Width();
			LEOPPHAPI static void Width(unsigned newWidth);

			[[nodiscard]]
			LEOPPHAPI static unsigned Height();
			LEOPPHAPI static void Height(unsigned newHeight);

			[[nodiscard]]
			LEOPPHAPI static float AspectRatio();

			[[nodiscard]]
			LEOPPHAPI static bool FullScreen();
			LEOPPHAPI static void FullScreen(bool newValue);

			[[nodiscard]]
			LEOPPHAPI static bool Vsync();
			LEOPPHAPI static void Vsync(bool newValue);

			[[nodiscard]]
			LEOPPHAPI static std::string_view Title();
			LEOPPHAPI static void Title(std::string newTitle);

			[[nodiscard]]
			LEOPPHAPI static float RenderMultiplier();
			LEOPPHAPI static void RenderMultiplier(float newMult);


			Window() = delete;
			Window(const Window& other) = delete;
			Window(Window&& other) = delete;

			Window& operator=(const Window& other) = delete;
			Window& operator=(Window&& other) = delete;

			~Window() = delete;
	};
}