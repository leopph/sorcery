#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Core.hpp"
#include "Event.hpp"
#include "Util.hpp"

#include <functional>


namespace leopph::platform
{
	LEOPPHAPI extern GuardedEventReference<Extent2D> OnWindowSize;
	LEOPPHAPI extern GuardedEventReference<> OnWindowFocusGain;
	LEOPPHAPI extern GuardedEventReference<> OnWindowFocusLoss;

	LEOPPHAPI [[nodiscard]] bool init_platform_support();
	LEOPPHAPI [[nodiscard]] bool process_platform_events();
	LEOPPHAPI void cleanup_platform_support();

	LEOPPHAPI [[nodiscard]] HWND get_hwnd();

	LEOPPHAPI [[nodiscard]] Extent2D get_window_current_client_area_size();
	LEOPPHAPI [[nodiscard]] Extent2D get_window_windowed_client_area_size();
	LEOPPHAPI void set_window_windowed_client_area_size(Extent2D size);

	LEOPPHAPI [[nodiscard]] bool is_window_borderless();
	LEOPPHAPI void set_window_borderless(bool borderless);

	LEOPPHAPI [[nodiscard]] bool is_window_minimizing_on_borderless_focus_loss();
	LEOPPHAPI void set_window_minimize_on_borderless_focus_loss(bool minimize);

	LEOPPHAPI [[nodiscard]] bool should_window_close();
	LEOPPHAPI void set_should_window_close(bool shouldClose);

	LEOPPHAPI [[nodiscard]] bool is_cursor_confined();
	LEOPPHAPI void confine_cursor(bool confine);

	LEOPPHAPI [[nodiscard]] bool is_cursor_hidden();
	LEOPPHAPI void hide_cursor(bool hide);

	LEOPPHAPI void SetEventHook(std::function<bool(HWND, UINT, WPARAM, LPARAM)> handler);

	namespace managedbindings
	{
		bool get_key(u8 key);
		bool get_key_down(u8 key);
		bool get_key_up(u8 key);
		Point2DI32 get_mouse_pos();
		Point2DI32 get_mouse_delta();
	}
}