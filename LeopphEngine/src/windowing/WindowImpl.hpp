#pragma once

#include "Window.hpp"
#include "../events/WindowEvent.hpp"
#include "../events/handling/EventReceiver.hpp"
#include "../math/Vector.hpp"

#include <memory>


namespace leopph::internal
{
	class WindowImpl : public Window, public EventReceiver<WindowEvent>
	{
		public:
			static auto Create() -> std::unique_ptr<WindowImpl>;

			virtual auto PollEvents() -> void = 0;
			virtual auto SwapBuffers() -> void = 0;
			virtual auto ClearColor(const Vector4& color) -> void = 0;
			virtual auto Clear() -> void = 0;
			[[nodiscard]] virtual auto ShouldClose() -> bool = 0;
			[[nodiscard]] virtual auto ClearColor() const -> const Vector4& = 0;

			WindowImpl(const WindowImpl& other) = delete;
			auto operator=(const WindowImpl& other) -> WindowImpl& = delete;

			WindowImpl(WindowImpl&& other) = delete;
			auto operator=(WindowImpl&& other) -> WindowImpl& = delete;

			~WindowImpl() override = default;

		protected:
			WindowImpl() = default;

		private:
			virtual auto InitKeys() -> void = 0;
	};
}
