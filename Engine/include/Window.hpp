#pragma once

#include "CursorState.hpp"
#include "LeopphApi.hpp"
#include "Types.hpp"

#include <string>
#include <string_view>
#include <vector>


struct GLFWwindow;


namespace leopph
{
	class Window
	{
		public:
			struct DisplayMode
			{
				u32 width;
				u32 height;
				u32 refreshRate;
			};


			[[nodiscard]] LEOPPHAPI u32 get_width() const;
			LEOPPHAPI void set_width(u32 width);


			[[nodiscard]] LEOPPHAPI u32 get_height() const;
			LEOPPHAPI void set_height(u32 height);


			[[nodiscard]] LEOPPHAPI f32 get_aspect_ratio() const;


			[[nodiscard]] LEOPPHAPI bool is_fullscreen() const;
			LEOPPHAPI void set_fullscreen(bool fullscreen);


			[[nodiscard]] LEOPPHAPI bool get_vsync() const;
			LEOPPHAPI void set_vsync(bool vsync);


			[[nodiscard]] LEOPPHAPI std::string_view get_title() const;
			LEOPPHAPI void set_title(std::string title);


			[[nodiscard]] LEOPPHAPI CursorState get_cursor_state() const;
			LEOPPHAPI void set_cursor_state(CursorState cursorState);


			[[nodiscard]] LEOPPHAPI std::vector<DisplayMode> get_supported_display_modes() const;


			[[nodiscard]] bool should_close() const;
			void set_should_close(bool shouldClose);


			void poll_events();
			void wait_events();
			void wait_events(f32 timeout);


			void swap_buffers();


			Window();

			Window(Window const& other) = delete;
			void operator=(Window const& other) = delete;

			Window(Window&& other) = delete;
			void operator=(Window&& other) = delete;

			~Window();

		private:
			void send_window_event() const;

			static void framebuffer_size_callback(GLFWwindow*, int width, int height);
			static void key_callback(GLFWwindow*, int key, int, int action, int);
			static void mouse_callback(GLFWwindow*, double x, double y);

			std::string mTitle{"LeopphEngine"};
			GLFWwindow* mHandle{nullptr};
			int mWidth{960};
			int mHeight{540};
			bool mFullscreen{false};
			bool mVsync{false};
	};
}
