#include "Input.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <hidusage.h>

#include <cassert>
#include <vector>


namespace leopph
{
	namespace
	{
		enum class Key : u8
		{
			Neutral = 0, Down = 1, Held = 2, Up = 3
		};

		Key gKeyboardState[256]{};
		Point2DI32 gMousePos{ 0, 0 };
		Point2DI32 gMouseDelta{ 0, 0 };
		HWND gMsgWindowHwnd{ nullptr };


		LRESULT CALLBACK msg_proc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam)
		{
			if (msg == WM_INPUT)
			{
				if (wparam == RIM_INPUT)
				{
					UINT dataSize;
					GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &dataSize, sizeof RAWINPUTHEADER);

					static std::vector<BYTE> data;
					data.reserve(dataSize);

					GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, data.data(), &dataSize, sizeof RAWINPUTHEADER);

					if (RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(data.data());
						raw->header.dwType == RIM_TYPEMOUSE)
					{
						gMouseDelta = { raw->data.mouse.lLastX, raw->data.mouse.lLastY };
						gMousePos.x += gMouseDelta.x;
						gMousePos.y += gMouseDelta.y;
					}

					DefWindowProcW(hwnd, msg, wparam, lparam);
				}

				return 0;
			}

			return DefWindowProcW(hwnd, msg, wparam, lparam);
		}
	}


	bool init_input_system()
	{
		WNDCLASSEXW wx{};
		wx.cbSize = sizeof(wx);
		wx.lpfnWndProc = msg_proc;
		wx.hInstance = GetModuleHandleW(nullptr);
		wx.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		wx.lpszClassName = L"LeopphInputHandlerWindow";

		if (!RegisterClassExW(&wx))
		{
			MessageBoxW(nullptr, L"Failed to register input handler window class.", L"Error", MB_ICONERROR);
			return false;
		}

		gMsgWindowHwnd = CreateWindowExW(0, wx.lpszClassName, wx.lpszClassName, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wx.hInstance, nullptr);

		if (!gMsgWindowHwnd)
		{
			MessageBoxW(nullptr, L"Failed to create input handler window.", L"Error", MB_ICONERROR);
			return false;
		}

		RAWINPUTDEVICE const rid
		{
			.usUsagePage = HID_USAGE_PAGE_GENERIC,
			.usUsage = HID_USAGE_GENERIC_MOUSE,
			.dwFlags = RIDEV_NOLEGACY,
			.hwndTarget = gMsgWindowHwnd
		};

		if (!RegisterRawInputDevices(&rid, 1, sizeof RAWINPUTDEVICE))
		{
			MessageBoxW(nullptr, L"Failed to register raw input devices.", L"Error", MB_ICONERROR);
			return false;
		}

		return true;
	}


	bool update_input_system()
	{
		// Null out delta in case a WM_INPUT is not triggered this frame to change it
		gMouseDelta = { 0, 0 };

		MSG msg;

		while (PeekMessageW(&msg, gMsgWindowHwnd, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

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


	void cleanup_input_system()
	{
		DestroyWindow(gMsgWindowHwnd);
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