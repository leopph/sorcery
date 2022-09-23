#pragma once

#include "Api.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <memory>
#include <functional>


namespace leopph
{
	class Window
	{
		private:
			explicit Window(HWND hwnd);

		public:
			LEOPPHAPI ~Window();

			Window(Window const& other) = delete;
			Window(Window&& other) noexcept = delete;

			Window& operator=(Window const& other) = delete;
			Window& operator=(Window&& other) noexcept = delete;

			LEOPPHAPI [[nodiscard]] static std::unique_ptr<Window> create(HINSTANCE hInstance);

			LEOPPHAPI [[nodiscard]] bool should_close() const;

			LEOPPHAPI void set_size_callback(std::function<void(WORD, WORD)> callback);

			LEOPPHAPI void process_messages() const;

			LEOPPHAPI HWND get_hwnd() const;

			LEOPPHAPI void set_display_mode(bool borderless);
			LEOPPHAPI bool is_borderless() const;

		private:
			static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

			static bool sRegistered;

			HWND mHwnd;
			bool mShouldClose{false};
			bool mBorderless{true};
			std::function<void(WORD, WORD)> mSizeCallback;
	};
}
