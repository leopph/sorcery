#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "Core.hpp"
#include "Event.hpp"
#include "Util.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <memory>


namespace leopph {
	enum class Key : u8;


	class Window {
	private:
		static auto CALLBACK WindowProc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) noexcept ->LRESULT;
		auto ApplyClientAreaSize() noexcept -> void;

		wchar_t const constexpr static* WND_CLASS_NAME{ L"LeopphEngine" };
		DWORD constexpr static WND_WINDOWED_STYLE{ WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME };
		DWORD constexpr static WND_BORDLERLESS_STYLE{ WS_POPUP };
		HCURSOR const inline static DEFAULT_CURSOR{ LoadCursorW(nullptr, IDC_ARROW) };

		std::unique_ptr < std::remove_pointer_t<HWND>, decltype([](HWND const hwnd) { DestroyWindow(hwnd); }) > mHwnd{ nullptr };
		bool mQuitSignaled{ false };
		DWORD mCurrentStyle{ WND_BORDLERLESS_STYLE };
		bool mMinimizeOnBorderlessFocusLoss{ false };
		bool mBorderless{ true };
		Extent2D<u32> mWindowedClientAreaSize{ .width = 1280, .height = 720 };
		Event<Extent2D<u32>> mOnSizeEvent;
		Event<> mOnFocusGainEvent;
		Event<> mOnFocusLossEvent;
		Point2D<i32> mMousePos{ 0, 0 };
		Point2D<i32> mMouseDelta{ 0, 0 };
		bool mConfineCursor{ false };
		bool mHideCursor{ false };
		bool mInFocus{ true };
		std::function<bool(HWND, UINT, WPARAM, LPARAM)> mEventHook{ nullptr };
		bool mIgnoreManagedRequests{ false };

	public:
		GuardedEventReference<Extent2D<u32>> OnWindowSize{ mOnSizeEvent };
		GuardedEventReference<> OnWindowFocusGain{ mOnFocusGainEvent };
		GuardedEventReference<> OnWindowFocusLoss{ mOnFocusLossEvent };

		LEOPPHAPI Window();

		LEOPPHAPI auto ProcessEvents() -> void;

		[[nodiscard]] LEOPPHAPI auto GetHandle() const noexcept -> HWND;

		[[nodiscard]] LEOPPHAPI auto GetCurrentClientAreaSize() const noexcept -> Extent2D<u32>;
		[[nodiscard]] LEOPPHAPI auto GetWindowedClientAreaSize() const noexcept -> Extent2D<u32>;
		LEOPPHAPI auto SetWindowedClientAreaSize(Extent2D<u32> size) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto IsBorderless() const noexcept -> bool;
		LEOPPHAPI auto SetBorderless(bool borderless) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool;
		LEOPPHAPI auto SetWindowMinimizeOnBorderlessFocusLoss(bool minimize) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto IsQuitSignaled() const noexcept -> bool;
		LEOPPHAPI auto SetQuitSignal(bool quit) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto IsCursorConfined() const noexcept -> bool;
		LEOPPHAPI auto SetCursorConfinement(bool confine) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto IsCursorHidden() const noexcept -> bool;
		LEOPPHAPI auto SetCursorHiding(bool hide) noexcept -> void;

		LEOPPHAPI auto SetEventHook(std::function<bool(HWND, UINT, WPARAM, LPARAM)> handler) noexcept -> void;

		[[nodiscard]] LEOPPHAPI auto GetMousePosition() const noexcept -> Point2D<i32>;
		[[nodiscard]] LEOPPHAPI auto GetMouseDelta() const noexcept -> Point2D<i32>;

		[[nodiscard]] LEOPPHAPI auto IsIgnoringManagedRequests() const noexcept -> bool;
		LEOPPHAPI auto SetIgnoreManagedRequests(bool ignore) noexcept -> void;
	};


	[[nodiscard]] LEOPPHAPI auto GetKey(Key key) noexcept -> bool;
	[[nodiscard]] LEOPPHAPI auto GetKeyDown(Key key) noexcept -> bool;
	[[nodiscard]] LEOPPHAPI auto GetKeyUp(Key key) noexcept -> bool;

	[[nodiscard]] LEOPPHAPI std::string WideToUtf8(std::wstring_view wstr);

	[[nodiscard]] LEOPPHAPI std::wstring_view GetExecutablePath() noexcept;

	LEOPPHAPI auto DisplayError(std::string_view msg) noexcept -> void;


	namespace managedbindings {
		template<Window const& WindowInstance>
		auto IsWindowBorderLess() noexcept -> int {
			return WindowInstance.IsBorderless();
		}


		template<Window& WindowInstance>
		auto SetWindowBorderless(int const borderless) noexcept -> void {
			if (!WindowInstance.IsIgnoringManagedRequests()) {
				WindowInstance.SetBorderless(borderless);
			}
		}


		template<Window const& WindowInstance>
		auto IsWindowMinimizingOnBorderlessFocusLoss() noexcept -> int {
			return WindowInstance.IsMinimizingOnBorderlessFocusLoss();
		}


		template<Window& WindowInstance>
		auto SetWindowMinimizeOnBorderlessFocusLoss(int const minimize) noexcept -> void {
			if (!WindowInstance.IsIgnoringManagedRequests()) {
				WindowInstance.SetWindowMinimizeOnBorderlessFocusLoss(minimize);
			}
		}


		template<Window& WindowInstance>
		auto SetWindowQuitSignal(int const quit) noexcept -> void {
			if (!WindowInstance.IsIgnoringManagedRequests()) {
				WindowInstance.SetQuitSignal(quit);
			}
		}
	}


	enum class Key : u8 {
		LeftMouseButton = 0x01,
		RightMouseButton = 0x02,
		Cancel = 0x03,
		MiddleMouseButton = 0x04,
		XButton1 = 0x05,
		XButton2 = 0x06,
		Backspace = 0x08,
		Tab = 0x09,
		Clear = 0x0C,
		Enter = 0x0D,
		Shift = 0x10,
		Control = 0x11,
		Alt = 0x12,
		Pause = 0x13,
		CapsLock = 0x14,
		Escape = 0x1B,
		Space = 0x20,
		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,
		LeftArrow = 0x25,
		UpArrow = 0x26,
		RightArrow = 0x27,
		DownArrow = 0x28,
		Select = 0x29,
		Print = 0x2A,
		Execute = 0x2B,
		PrintScreen = 0x2C,
		Insert = 0x2D,
		Delete = 0x2E,
		Help = 0x2F,
		Zero = 0x30,
		One = 0x31,
		Two = 0x32,
		Three = 0x33,
		Four = 0x34,
		Five = 0x35,
		Six = 0x36,
		Seven = 0x37,
		Eight = 0x38,
		Nine = 0x39,
		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,
		LeftWindows = 0x5B,
		RightWindow = 0x5C,
		Apps = 0x5D,
		Sleep = 0x5F,
		NumPad0 = 0x60,
		NumPad1 = 0x61,
		NumPad2 = 0x62,
		NumPad3 = 0x63,
		NumPad4 = 0x64,
		NumPad5 = 0x65,
		NumPad6 = 0x66,
		NumPad7 = 0x67,
		NumPad8 = 0x68,
		NumPad9 = 0x69,
		Multiply = 0x6A,
		Add = 0x6B,
		Separator = 0x6C,
		Subtract = 0x6D,
		Decimal = 0x6E,
		Divide = 0x6F,
		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7A,
		F12 = 0x7B,
		F13 = 0x7C,
		F14 = 0x7D,
		F15 = 0x7E,
		F16 = 0x7F,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,
		NumLock = 0x90,
		ScrollLock = 0x91,
		LeftShift = 0xA0,
		RightShift = 0xA1,
		LeftControl = 0xA2,
		RightControl = 0xA3,
		LeftAlt = 0xA4,
		RightAlt = 0xA5,
	};
}