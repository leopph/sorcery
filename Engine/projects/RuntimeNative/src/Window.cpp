#include "Window.hpp"


namespace leopph
{
	bool Window::sClassRegistered{false};
	wchar_t const* const Window::sClassName{L"LeopphEngine"};
	DWORD const Window::sInitialStyle{WS_OVERLAPPEDWINDOW};


	std::unique_ptr<Window> Window::Create()
	{
		if (!sClassRegistered)
		{
			WNDCLASSW const wndClass
			{
				.style = 0,
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

		HWND const hwnd = CreateWindowExW(0, sClassName, sClassName, sInitialStyle, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);

		if (!hwnd)
		{
			MessageBoxW(nullptr, L"Failed to create window.\n", L"Error", MB_ICONERROR);
			return nullptr;
		}

		Window* window = new Window{hwnd};

		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

		return std::unique_ptr<Window>{window};
	}


	void Window::process_events()
	{
		MSG msg;

		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				mShouldClose = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
	}


	bool Window::should_close() const
	{
		return mShouldClose;
	}


	Window::Window(HWND const hwnd) :
		mHwnd{hwnd}
	{}


	LRESULT CALLBACK Window::window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
	{
		Window* const self = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));

		switch (msg)
		{
			case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}

			case WM_SIZE:
			{
				self->mOnSizeEvent.invoke({LOWORD(lparam), HIWORD(lparam)});
				return 0;
			}

			default:
			{
				return DefWindowProcW(hwnd, msg, wparam, lparam);
			}
		}		
	}


	Window::~Window()
	{
		DestroyWindow(mHwnd);
	}


	void Window::set_client_area_size(i32 const width, i32 const height)
	{
		RECT rect
		{
			.left = 0,
			.top = 0,
			.right = width,
			.bottom = height
		};

		AdjustWindowRect(&rect, mCurrentStyle, FALSE);
		SetWindowPos(mHwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, 0);
	}


	HWND Window::get_hwnd() const
	{
		return mHwnd;
	}


	void Window::show() const
	{
		ShowWindow(mHwnd, SW_SHOW);
	}


	void Window::hide() const
	{
		ShowWindow(mHwnd, SW_SHOW);
	}
}