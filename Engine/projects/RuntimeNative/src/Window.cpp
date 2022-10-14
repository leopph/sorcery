#include "Window.hpp"


namespace leopph
{
	namespace
	{
		Window* sLastInstance{ nullptr };
	}


	bool Window::sClassRegistered{ false };
	wchar_t const* const Window::sClassName{ L"LeopphEngine" };
	DWORD const Window::sWindowedModeStyle{ WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME };
	DWORD const Window::sBorderlessModeStyle{ WS_POPUP };


	std::unique_ptr<Window> Window::Create()
	{
		if (!sClassRegistered)
		{
			WNDCLASSW const wndClass
			{
				.style = CS_VREDRAW | CS_HREDRAW,
				.lpfnWndProc = window_proc,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = GetModuleHandleW(nullptr),
				.hIcon = nullptr,
				.hCursor = LoadCursorW(nullptr, IDC_ARROW),
				.hbrBackground = nullptr,
				.lpszMenuName = nullptr,
				.lpszClassName = sClassName,
			};

			if (!RegisterClassW(&wndClass))
			{
				MessageBoxW(nullptr, L"Failed to register window class.\n", L"Error", MB_ICONERROR);
				return nullptr;
			};
		}

		HWND const hwnd = CreateWindowExW(0, sClassName, sClassName, sBorderlessModeStyle, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);

		if (!hwnd)
		{
			MessageBoxW(nullptr, L"Failed to create window.\n", L"Error", MB_ICONERROR);
			return nullptr;
		}

		ShowWindow(hwnd, SW_SHOWDEFAULT);

		Window* window = new Window{ hwnd };

		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

		sLastInstance = window;

		return std::unique_ptr<Window>{window};
	}


	void Window::process_events()
	{
		MSG msg;

		while (PeekMessageW(&msg, mHwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);

		}
	}


	bool Window::should_close() const
	{
		return mShouldClose;
	}


	Window::Window(HWND const hwnd) :
		mHwnd{ hwnd }
	{}


	LRESULT CALLBACK Window::window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
	{
		Window* const self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		switch (msg)
		{
			case WM_CLOSE:
			{
				self->mShouldClose = true;
				return 0;
			}

			case WM_SIZE:
			{
				if (self)
				{
					self->mOnSizeEvent.invoke({ LOWORD(lparam), HIWORD(lparam) });
				}
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

			case WM_ACTIVATEAPP:
			{
				if (wparam == FALSE && self->mBorderless && self->mMinimizeOnBorderlessFocusLoss)
				{
					ShowWindow(hwnd, SW_MINIMIZE);
					return 0;
				}
				break;
			}
		}

		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}


	void Window::apply_client_area_size()
	{
		RECT rect
		{
			.left = 0,
			.top = 0,
			.right = static_cast<LONG>(mWindowedClientAreaSize.width),
			.bottom = static_cast<LONG>(mWindowedClientAreaSize.height)
		};

		AdjustWindowRect(&rect, mCurrentStyle, FALSE);
		SetWindowPos(mHwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED);
	}


	Window::~Window()
	{
		DestroyWindow(mHwnd);
	}


	HWND Window::get_hwnd() const
	{
		return mHwnd;
	}


	Extent2D Window::get_current_client_area_size() const
	{
		RECT rect;
		GetClientRect(mHwnd, &rect);
		return { static_cast<u32>(rect.right), static_cast<u32>(rect.bottom) };
	}


	Extent2D Window::get_windowed_client_area_size() const
	{
		return mWindowedClientAreaSize;
	}


	void Window::set_windowed_client_area_size(Extent2D const size)
	{
		mWindowedClientAreaSize = size;

		if (!mBorderless)
		{
			apply_client_area_size();
		}
	}


	bool Window::is_borderless() const
	{
		return mBorderless;
	}


	void Window::set_borderless(bool const borderless)
	{
		if (borderless == mBorderless)
		{
			return;
		}

		mBorderless = borderless;

		if (mBorderless)
		{
			SetWindowLongPtrW(mHwnd, GWL_STYLE, sBorderlessModeStyle | WS_VISIBLE);
			SetWindowPos(mHwnd, nullptr, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLongPtrW(mHwnd, GWL_STYLE, sWindowedModeStyle | WS_VISIBLE);
			apply_client_area_size();
		}
	}


	bool Window::is_minimizing_on_borderless_focus_loss() const
	{
		return mMinimizeOnBorderlessFocusLoss;
	}


	void Window::set_minimize_on_borderless_focus_loss(bool const minimize)
	{
		mMinimizeOnBorderlessFocusLoss = minimize;
	}


	namespace managedbindings
	{
		Extent2D get_window_current_client_area_size()
		{
			return sLastInstance->get_current_client_area_size();
		}


		Extent2D get_window_windowed_client_area_size()
		{
			return sLastInstance->get_windowed_client_area_size();
		}


		void set_window_windowed_client_area_size(Extent2D const size)
		{
			sLastInstance->set_windowed_client_area_size(size);
		}


		i32 is_window_borderless()
		{
			return sLastInstance->is_borderless();
		}


		void set_window_borderless(i32 const borderless)
		{
			sLastInstance->set_borderless(borderless);
		}


		i32 is_window_minimizing_on_borderless_focus_loss()
		{
			return sLastInstance->is_minimizing_on_borderless_focus_loss();
		}


		void set_window_minimize_on_borderless_focus_loss(i32 const minimize)
		{
			sLastInstance->set_minimize_on_borderless_focus_loss(minimize);
		}
	}
}