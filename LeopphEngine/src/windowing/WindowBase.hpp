#pragma once

#include "../input/CursorState.hpp"
#include "../misc/Color.hpp"

#include <string>
#include <string_view>


namespace leopph::internal
{
	class WindowBase
	{
		public:
			static auto Get(unsigned width = 1280u, unsigned height = 720u,
			                const std::string& title = "LeopphEngine Application", bool fullscreen = false) -> WindowBase&;
			static auto Destroy() -> void;

			[[nodiscard]]
			virtual auto Width() const -> unsigned = 0;
			virtual auto Width(unsigned newWidth) -> void = 0;

			[[nodiscard]]
			virtual auto Height() const -> unsigned = 0;
			virtual auto Height(unsigned newHeight) -> void = 0;

			[[nodiscard]]
			auto AspectRatio() const -> float;

			[[nodiscard]]
			virtual auto Fullscreen() const -> bool = 0;
			virtual auto Fullscreen(bool newValue) -> void = 0;

			[[nodiscard]]
			virtual auto Vsync() const -> bool = 0;
			virtual auto Vsync(bool newValue) -> void = 0;

			[[nodiscard]]
			virtual auto Title() const -> std::string_view = 0;
			virtual auto Title(std::string newTitle) -> void = 0;

			[[nodiscard]]
			virtual auto Background() const -> const Color& = 0;
			virtual auto Background(const Color& color) -> void = 0;

			[[nodiscard]]
			virtual auto CursorMode() const -> CursorState = 0;
			virtual auto CursorMode(CursorState newState) -> void = 0;

			[[nodiscard]]
			virtual auto RenderMultiplier() -> float = 0;
			virtual auto RenderMultiplier(float newMult) -> void = 0;

			virtual auto PollEvents() -> void = 0;
			virtual auto SwapBuffers() -> void = 0;
			virtual auto ShouldClose() -> bool = 0;
			virtual auto Clear() -> void = 0;

			WindowBase(const WindowBase& other) = delete;
			WindowBase(WindowBase&& other) = delete;

			auto operator=(const WindowBase& other) -> WindowBase& = delete;
			auto operator=(WindowBase&& other) -> WindowBase& = delete;

			virtual ~WindowBase() = 0;

		protected:
			WindowBase() = default;

		private:
			virtual auto InitKeys() -> void = 0;

			static WindowBase* s_Instance;
	};
}
