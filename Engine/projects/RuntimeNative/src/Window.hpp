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
		static DWORD const sWindowedModeStyle;
		static DWORD const sBorderlessModeStyle;


		HWND mHwnd;
		bool mShouldClose{ false };
		DWORD mCurrentStyle{ sWindowedModeStyle };
		Event<Extent2D> mOnSizeEvent;
		bool mMinimizeOnBorderlessFocusLoss{ false };
		bool mBorderless{ true };
		Extent2D mWindowedClientAreaSize{ .width = 1280, .height = 720 };


	public:
		GuardedEventReference<Extent2D> OnSizeEvent{ mOnSizeEvent };

		LEOPPHAPI [[nodiscard]] static std::unique_ptr<Window> Create();

		LEOPPHAPI void process_events();
		LEOPPHAPI [[nodiscard]] bool should_close() const;

		LEOPPHAPI ~Window();

		LEOPPHAPI HWND get_hwnd() const;

		LEOPPHAPI Extent2D get_current_client_area_size() const;
		LEOPPHAPI Extent2D get_windowed_client_area_size() const;
		LEOPPHAPI void set_windowed_client_area_size(Extent2D size);

		LEOPPHAPI [[nodiscard]] bool is_borderless() const;
		LEOPPHAPI void set_borderless(bool borderless);

		LEOPPHAPI [[nodiscard]] bool is_minimizing_on_borderless_focus_loss() const;
		LEOPPHAPI void set_minimize_on_borderless_focus_loss(bool minimize);

	private:
		Window(HWND hwnd);

		static LRESULT CALLBACK window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam);
		void apply_client_area_size();
	};


	namespace detail
	{
		[[nodiscard]] Extent2D get_window_current_client_area_size();
		[[nodiscard]] Extent2D get_window_windowed_client_area_size();
		void set_window_windowed_client_area_size(Extent2D size);

		[[nodiscard]] i32 is_window_borderless();
		void set_window_borderless(i32 borderless);

		[[nodiscard]] i32 is_window_minimizing_on_borderless_focus_loss();
		void set_window_minimize_on_borderless_focus_loss(i32 minimize);
	}
}