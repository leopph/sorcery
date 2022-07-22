#pragma once

#include "Vector.hpp"
#include "Window.hpp"

#include <memory>


namespace leopph::internal
{
	class WindowImpl : public Window
	{
		public:
			static std::unique_ptr<WindowImpl> Create();

			[[nodiscard]]
			virtual bool ShouldClose() = 0;
			virtual void ShouldClose(bool val) = 0;

			[[nodiscard]]
			virtual Vector4 const& ClearColor() const = 0;
			virtual void ClearColor(Vector4 const& color) = 0;

			virtual void PollEvents() = 0;
			virtual void SwapBuffers() = 0;
			virtual void Clear() = 0;

			WindowImpl(WindowImpl const& other) = delete;
			WindowImpl& operator=(WindowImpl const& other) = delete;

			WindowImpl(WindowImpl&& other) = delete;
			WindowImpl& operator=(WindowImpl&& other) = delete;

			~WindowImpl() override = default;

		protected:
			WindowImpl() = default;
	};
}
