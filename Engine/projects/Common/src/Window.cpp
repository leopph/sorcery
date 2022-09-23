#include "Window.hpp"

#include <utility>



namespace leopph
{
	Window::Window(HWND const hwnd) :
		mHwnd{hwnd}
	{
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		ShowWindow(hwnd, SW_SHOWDEFAULT);
	}



	Window::~Window()
	{
		DestroyWindow(mHwnd);
	}



	std::unique_ptr<Window> Window::create(HINSTANCE hInstance)
	{
		auto const* const WIN_CLASS_NAME = L"Leopph";

		if (!sRegistered)
		{
			WNDCLASSEXW const windowClass
			{
				.cbSize = sizeof(WNDCLASSEX),
				.style = CS_HREDRAW | CS_VREDRAW,
				.lpfnWndProc = &window_proc,
				.cbClsExtra = 0,
				.cbWndExtra = 0,
				.hInstance = hInstance,
				.hIcon = LoadIconW(nullptr, IDI_APPLICATION),
				.hCursor = LoadCursorW(nullptr, IDC_ARROW),
				.hbrBackground = nullptr,
				.lpszMenuName = nullptr,
				.lpszClassName = WIN_CLASS_NAME,
				.hIconSm = LoadIconW(nullptr, IDI_APPLICATION)
			};

			if (!RegisterClassExW(&windowClass))
			{
				MessageBoxW(nullptr, L"Failed to register window class.", L"Error", MB_ICONERROR | MB_OK);
				return nullptr;
			}

			sRegistered = true;
		}

		auto const hwnd = CreateWindowExW(0, WIN_CLASS_NAME, L"MyWindow", WS_POPUP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, hInstance, nullptr);

		if (!hwnd)
		{
			MessageBoxW(nullptr, L"Failed to create window.", L"Error", MB_ICONERROR | MB_OK);
			return nullptr;
		}

		return std::unique_ptr<Window>(new Window{hwnd});
	}



	bool Window::should_close() const
	{
		return mShouldClose;
	}



	void Window::set_size_callback(std::function<void(WORD, WORD)> callback)
	{
		mSizeCallback = std::move(callback);
	}



	void Window::process_messages() const
	{
		MSG msg;

		while (PeekMessageW(&msg, mHwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}



	HWND Window::get_hwnd() const
	{
		return mHwnd;
	}



	void Window::set_display_mode(bool const borderless)
	{
		if (mBorderless == borderless)
		{
			return;
		}

		mBorderless = !mBorderless;

		if (!mBorderless)
		{
			DWORD constexpr style{WS_BORDER | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_SYSMENU | WS_VISIBLE};
			SetWindowLongPtrW(mHwnd, GWL_STYLE, style);
			RECT rect
			{
				.left = 0,
				.top = 0,
				.right = 1280,
				.bottom = 720
			};
			AdjustWindowRect(&rect, style & ~WS_OVERLAPPED & ~WS_SYSMENU, FALSE);
			SetWindowPos(mHwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED);
		}
		else
		{
			SetWindowLongPtrW(mHwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowPos(mHwnd, nullptr, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
		}
	}



	bool Window::is_borderless() const
	{
		return mBorderless;
	}



	LRESULT Window::window_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
	{
		if (auto* const instance = reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)))
		{
			switch (msg)
			{
				case WM_CLOSE:
				{
					instance->mShouldClose = true;
					return 0;
				}

				case WM_SIZE:
				{
					if (instance->mSizeCallback)
					{
						instance->mSizeCallback(LOWORD(lparam), HIWORD(lparam));
					}
					return 0;
				}

				case WM_SYSKEYDOWN:
				{
					if (wparam == VK_RETURN && (lparam & 0x60000000) == 0x20000000)
					{
						instance->set_display_mode(!instance->mBorderless);
						return 0;
					}
					break;
				}

				case WM_SYSCOMMAND:
				{
					if (wparam == SC_KEYMENU)
					{
						return 0;
					}
					break;
				}
			}
		}

		return DefWindowProcW(hwnd, msg, wparam, lparam);
	}



	bool Window::sRegistered{false};
}
