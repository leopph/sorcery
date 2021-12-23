#pragma once

#include "../input/cursorstate.h"
#include "../misc/Color.hpp"

#include <string>
#include <string_view>


namespace leopph::internal
{
	class WindowBase
	{
	public:
		static WindowBase& Get(unsigned width = 1280u, unsigned height = 720u,
			const std::string& title = "LeopphEngine Application", bool fullscreen = false);
		static void Destroy();

		[[nodiscard]]
		virtual unsigned Width() const = 0;
		virtual void Width(unsigned newWidth) = 0;

		[[nodiscard]]
		virtual unsigned Height() const = 0;
		virtual void Height(unsigned newHeight) = 0;

		[[nodiscard]]
		float AspectRatio() const;

		[[nodiscard]]
		virtual bool Fullscreen() const = 0;
		virtual void Fullscreen(bool newValue) = 0;

		[[nodiscard]]
		virtual bool Vsync() const = 0;
		virtual void Vsync(bool newValue) = 0;

		[[nodiscard]]
		virtual std::string_view Title() const = 0;
		virtual void Title(std::string newTitle) = 0;

		[[nodiscard]]
		virtual const Color& Background() const = 0;
		virtual void Background(const Color& color) = 0;

		[[nodiscard]]
		virtual CursorState CursorMode() const = 0;
		virtual void CursorMode(CursorState newState) = 0;

		[[nodiscard]]
		virtual float RenderMultiplier() = 0;
		virtual void RenderMultiplier(float newMult) = 0;

		virtual void PollEvents() = 0;
		virtual void SwapBuffers() = 0;
		virtual bool ShouldClose() = 0;
		virtual void Clear() = 0;


		WindowBase(const WindowBase& other) = delete;
		WindowBase(WindowBase&& other) = delete;

		WindowBase& operator=(const WindowBase& other) = delete;
		WindowBase& operator=(WindowBase&& other) = delete;

		virtual ~WindowBase() = 0;


	protected:
		WindowBase() = default;
		
	private:
		virtual void InitKeys() = 0;
		
		static WindowBase* s_Instance;
	};
}