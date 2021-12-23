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
			LEOPPHAPI static auto Width() -> unsigned;
			LEOPPHAPI static auto Width(unsigned newWidth) -> void;

			[[nodiscard]]
			LEOPPHAPI static auto Height() -> unsigned;
			LEOPPHAPI static auto Height(unsigned newHeight) -> void;

			[[nodiscard]]
			LEOPPHAPI static auto AspectRatio() -> float;

			[[nodiscard]]
			LEOPPHAPI static auto FullScreen() -> bool;
			LEOPPHAPI static auto FullScreen(bool newValue) -> void;

			[[nodiscard]]
			LEOPPHAPI static auto Vsync() -> bool;
			LEOPPHAPI static auto Vsync(bool newValue) -> void;

			[[nodiscard]]
			LEOPPHAPI static auto Title() -> std::string_view;
			LEOPPHAPI static auto Title(std::string newTitle) -> void;

			[[nodiscard]]
			LEOPPHAPI static auto RenderMultiplier() -> float;
			LEOPPHAPI static auto RenderMultiplier(float newMult) -> void;

			Window() = delete;
			Window(const Window& other) = delete;
			Window(Window&& other) = delete;

			auto operator=(const Window& other) -> Window& = delete;
			auto operator=(Window&& other) -> Window& = delete;

			~Window() = delete;
	};
}
