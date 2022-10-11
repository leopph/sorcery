#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Core.hpp"
#include "Extent2D.hpp"
#include "Event.hpp"

#include <memory>


namespace leopph
{
	class Window
	{
	private:
		static bool sClassRegistered;
		static wchar_t const* const sClassName;
		static DWORD const sInitialStyle;


		HWND mHwnd;
		bool mShouldClose{false};
		DWORD mCurrentStyle{sInitialStyle};
		Event<Extent2D> mOnSizeEvent;


	public:
		GuardedEventReference<Extent2D> OnSizeEvent{mOnSizeEvent};

		LEOPPHAPI [[nodiscard]] static std::unique_ptr<Window> Create();

		LEOPPHAPI void process_events();
		LEOPPHAPI [[nodiscard]] bool should_close() const;

		LEOPPHAPI ~Window();

		LEOPPHAPI void set_client_area_size(i32 width, i32 height);

		LEOPPHAPI HWND get_hwnd() const;

		LEOPPHAPI void show() const;
		LEOPPHAPI void hide() const;

	private:
		Window(HWND hwnd);

		static LRESULT CALLBACK window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam);
	};
}