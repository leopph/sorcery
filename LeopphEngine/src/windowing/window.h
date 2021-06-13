#pragma once

#include <string>
#include "../api/leopphapi.h"

namespace leopph::impl
{
	class Window
	{
	public:
		// TODO separate header
		enum class LEOPPHAPI CursorState
		{
			Shown,
			Hidden,
			Disabled
		};

		LEOPPHAPI static Window& Get(unsigned width = 1280u, unsigned height = 720u,
			const std::string& title = "Window", bool fullscreen = false);
		LEOPPHAPI static void Destroy();

		LEOPPHAPI virtual unsigned Width() const;
		LEOPPHAPI virtual void Width(unsigned newWidth);

		LEOPPHAPI virtual unsigned Height() const;
		LEOPPHAPI virtual void Height(unsigned newHeight);

		LEOPPHAPI float AspectRatio() const;

		LEOPPHAPI bool Fullscreen() const;

		LEOPPHAPI virtual void PollEvents() = 0;
		LEOPPHAPI virtual void SwapBuffers() = 0;
		LEOPPHAPI virtual bool ShouldClose() = 0;
		LEOPPHAPI virtual void Clear() = 0;

		LEOPPHAPI virtual CursorState CursorMode() const = 0;
		LEOPPHAPI virtual void CursorMode(CursorState newState) = 0;

	protected:
		Window(unsigned width, unsigned height,
			const std::string& title, bool fullscreen);
		virtual ~Window() = default;
		
	private:
		static Window* s_Instance;

		unsigned m_Width;
		unsigned m_Height;
		std::string m_Title;
		bool m_Fullscreen; 
	};
}