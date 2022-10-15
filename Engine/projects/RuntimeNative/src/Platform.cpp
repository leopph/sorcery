#include "Platform.hpp"

#include <hidusage.h>
#include <cassert>
#include <memory>


namespace leopph::platform
{
	enum class Key : u8
	{
		Neutral = 0, Down = 1, Held = 2, Up = 3
	};


	namespace
	{
		wchar_t const* const WND_CLASS_NAME{ L"LeopphEngine" };
		DWORD constexpr WND_WINDOWED_STYLE{ WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME };
		DWORD constexpr WND_BORDLERLESS_STYLE{ WS_POPUP };
		HCURSOR const DEFAULT_CURSOR{ LoadCursorW(nullptr, IDC_ARROW) };

		HWND gHwnd{ nullptr };
		bool gWndShouldClose{ false };
		DWORD gWndStyle{ WND_BORDLERLESS_STYLE };
		bool gWndMinimizeOnBorderlessFocusLoss{ false };
		bool gWndBorderless{ true };
		Extent2D gWndWindowedClientAreaSize{ .width = 1280, .height = 720 };
		Event<Extent2D> gWndOnSizeEvent;
		Event<> gWndOnFocusGainEvent;
		Event<> gWndOnFocusLossEvent;
		Key gKeyboardState[256]{};
		Point2DI32 gMousePos{ 0, 0 };
		Point2DI32 gMouseDelta{ 0, 0 };
		bool gConfineCursor{ false };
		bool gHideCursor{ false };
		bool gInFocus{ true };
	}


	GuardedEventReference<Extent2D> OnWindowSize{ gWndOnSizeEvent };
	GuardedEventReference<> OnWindowFocusGain{ gWndOnFocusGainEvent };
	GuardedEventReference<> OnWindowFocusLoss{ gWndOnFocusLossEvent };


	namespace
	{
		LRESULT CALLBACK wnd_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
		{
			switch (msg)
			{
				case WM_CLOSE:
				{
					gWndShouldClose = true;
					return 0;
				}

				case WM_SIZE:
				{
					gWndOnSizeEvent.invoke({ LOWORD(lparam), HIWORD(lparam) });
					return 0;
				}

				case WM_SYSCOMMAND:
				{
					if (wparam == SC_KEYMENU)
					{
						return 0;
					}
					break;
				}

				case WM_ACTIVATE:
				{
					if (LOWORD(wparam) == WA_INACTIVE)
					{
						if (gWndBorderless && gWndMinimizeOnBorderlessFocusLoss)
						{
							ShowWindow(hwnd, SW_MINIMIZE);
						}
						gInFocus = false;
						gWndOnFocusLossEvent.invoke();
					}
					else
					{
						gInFocus = true;
						gWndOnFocusGainEvent.invoke();
					}

					return 0;
				}

				case WM_INPUT:
				{
					if (wparam == RIM_INPUT)
					{
						UINT requiredSize;
						GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &requiredSize, sizeof RAWINPUTHEADER);

						auto static bufferSize = requiredSize;
						auto static buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);

						if (requiredSize > bufferSize)
						{
							bufferSize = requiredSize;
							buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);
						}

						GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, buffer.get(), &bufferSize, sizeof RAWINPUTHEADER);

						if (auto const* const raw = reinterpret_cast<RAWINPUT*>(buffer.get()); raw->header.dwType == RIM_TYPEMOUSE)
						{
							if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
							{
								gMouseDelta.x += raw->data.mouse.lLastX;
								gMouseDelta.y += raw->data.mouse.lLastY;
							}
						}

						DefWindowProcW(hwnd, msg, wparam, lparam);
					}

					return 0;
				}

				case WM_MOUSEMOVE:
				{
					if (gInFocus && gConfineCursor)
					{
						RECT rect;
						GetClientRect(gHwnd, &rect);
						POINT midPoint
						{
							.x = static_cast<LONG>(static_cast<float>(rect.right) / 2.0f),
							.y = static_cast<LONG>(static_cast<float>(rect.bottom) / 2.0f),
						};
						ClientToScreen(gHwnd, &midPoint);
						SetCursorPos(midPoint.x, midPoint.y);
					}

					SetCursor(gHideCursor ? nullptr : DEFAULT_CURSOR);						
				}
			}

			return DefWindowProcW(hwnd, msg, wparam, lparam);
		}


		void apply_client_area_size()
		{
			RECT rect
			{
				.left = 0,
				.top = 0,
				.right = static_cast<LONG>(gWndWindowedClientAreaSize.width),
				.bottom = static_cast<LONG>(gWndWindowedClientAreaSize.height)
			};

			AdjustWindowRect(&rect, gWndStyle, FALSE);
			SetWindowPos(gHwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED);
		}
	}


	bool init_platform_support()
	{
		WNDCLASSEXW const wx
		{
			.cbSize = sizeof WNDCLASSEXW,
			.lpfnWndProc = wnd_proc,
			.hInstance = GetModuleHandleW(nullptr),
			.lpszClassName = WND_CLASS_NAME
		};

		if (!RegisterClassExW(&wx))
		{
			MessageBoxW(nullptr, L"Failed to register window class.\n", L"Error", MB_ICONERROR);
			return false;
		}

		gHwnd = CreateWindowExW(0, wx.lpszClassName, wx.lpszClassName, WND_BORDLERLESS_STYLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wx.hInstance, nullptr);

		if (!gHwnd)
		{
			MessageBoxW(nullptr, L"Failed to create window.\n", L"Error", MB_ICONERROR);
			return false;
		}

		RAWINPUTDEVICE const rid
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = 0,
			.hwndTarget = gHwnd
		};

		if (!RegisterRawInputDevices(&rid, 1, sizeof RAWINPUTDEVICE))
		{
			MessageBoxW(nullptr, L"Failed to register raw input devices.", L"Error", MB_ICONERROR);
			return false;
		}

		ShowWindow(gHwnd, SW_SHOWNORMAL);
		return true;
	}


	bool process_platform_events()
	{
		// Null out delta in case a WM_INPUT is not triggered this frame to change it
		gMouseDelta = { 0, 0 };

		MSG msg;
		while (PeekMessageW(&msg, gHwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		// Apply the accumulated delta
		gMousePos.x += gMouseDelta.x;
		gMousePos.y += gMouseDelta.y;

		BYTE newState[256];

		if (!GetKeyboardState(newState))
		{
			MessageBoxW(nullptr, L"Failed to get keyboard state.", L"Error", MB_ICONERROR);
			return false;
		}

		for (int i = 0; i < 256; i++)
		{
			if (newState[i] & 0x80)
			{
				if (gKeyboardState[i] == Key::Down)
				{
					gKeyboardState[i] = Key::Held;
				}
				else if (gKeyboardState[i] != Key::Held)
				{
					gKeyboardState[i] = Key::Down;
				}
			}
			else
			{
				if (gKeyboardState[i] == Key::Up)
				{
					gKeyboardState[i] = Key::Neutral;
				}
				else
				{
					gKeyboardState[i] = Key::Up;
				}
			}
		}

		return true;
	}


	void cleanup_platform_support()
	{
		DestroyWindow(gHwnd);
	}


	HWND get_hwnd()
	{
		return gHwnd;
	}


	Extent2D get_window_current_client_area_size()
	{
		RECT rect;
		GetClientRect(gHwnd, &rect);
		return { static_cast<u32>(rect.right), static_cast<u32>(rect.bottom) };
	}


	Extent2D get_window_windowed_client_area_size()
	{
		return gWndWindowedClientAreaSize;
	}


	void set_window_windowed_client_area_size(Extent2D const size)
	{
		gWndWindowedClientAreaSize = size;

		if (!gWndBorderless)
		{
			apply_client_area_size();
		}
	}


	bool is_window_borderless()
	{
		return gWndBorderless;
	}


	void set_window_borderless(bool const borderless)
	{
		if (borderless == gWndBorderless)
		{
			return;
		}

		gWndBorderless = borderless;

		if (gWndBorderless)
		{
			SetWindowLongPtrW(gHwnd, GWL_STYLE, WND_BORDLERLESS_STYLE | WS_VISIBLE);
			SetWindowPos(gHwnd, nullptr, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLongPtrW(gHwnd, GWL_STYLE, WND_WINDOWED_STYLE | WS_VISIBLE);
			apply_client_area_size();
		}
	}


	bool is_window_minimizing_on_borderless_focus_loss()
	{
		return gWndMinimizeOnBorderlessFocusLoss;
	}


	void set_window_minimize_on_borderless_focus_loss(bool const minimize)
	{
		gWndMinimizeOnBorderlessFocusLoss = minimize;
	}


	bool should_window_close()
	{
		return gWndShouldClose;
	}


	void set_should_window_close(bool const shouldClose)
	{
		gWndShouldClose = shouldClose;
	}


	[[nodiscard]] bool is_cursor_confined()
	{
		return gConfineCursor;
	}


	void confine_cursor(bool const confine)
	{
		gConfineCursor = confine;
	}


	[[nodiscard]] bool is_cursor_hidden()
	{
		return gHideCursor;
	}


	void hide_cursor(bool const hide)
	{
		gHideCursor = hide;
	}


	namespace managedbindings
	{
		bool get_key(u8 const key)
		{
			return gKeyboardState[key] == Key::Down || gKeyboardState[key] == Key::Held;
		}


		bool get_key_down(u8 const key)
		{
			return gKeyboardState[key] == Key::Down;
		}


		bool get_key_up(u8 const key)
		{
			return gKeyboardState[key] == Key::Up;
		}


		Point2DI32 get_mouse_pos()
		{
			return gMousePos;
		}


		Point2DI32 get_mouse_delta()
		{
			return gMouseDelta;
		}
	}
}