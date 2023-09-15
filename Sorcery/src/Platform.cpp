#include "Platform.hpp"

#include <hidusage.h>
#include <cassert>
#include <memory>
#include <stdexcept>


namespace sorcery {
Window gWindow;


namespace {
enum class KeyState : std::uint8_t {
  Neutral = 0,
  Down    = 1,
  Held    = 2,
  Up      = 3
};


KeyState gKeyboardState[256]{};
}


auto CALLBACK Window::WindowProc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) noexcept -> LRESULT {
  if (auto const instance{reinterpret_cast<Window*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))}; instance) {
    if (instance->mEventHook && instance->mEventHook(hwnd, msg, wparam, lparam)) {
      return true;
    }

    switch (msg) {
      case WM_CLOSE: {
        instance->mQuitSignaled = true;
        return 0;
      }

      case WM_SIZE: {
        instance->mOnSizeEvent.invoke({LOWORD(lparam), HIWORD(lparam)});
        return 0;
      }

      case WM_SYSCOMMAND: {
        if (wparam == SC_KEYMENU) {
          return 0;
        }
        break;
      }

      case WM_ACTIVATE: {
        if (LOWORD(wparam) == WA_INACTIVE) {
          if (instance->mBorderless && instance->mMinimizeOnBorderlessFocusLoss) {
            ShowWindow(hwnd, SW_MINIMIZE);
          }
          instance->mInFocus = false;
          instance->mOnFocusLossEvent.invoke();
        } else {
          instance->mInFocus = true;
          instance->mOnFocusGainEvent.invoke();
        }
        return 0;
      }

      case WM_INPUT: {
        if (wparam == RIM_INPUT) {
          UINT requiredSize;
          GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &requiredSize, sizeof(RAWINPUTHEADER));

          auto static bufferSize = requiredSize;
          auto static buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);

          if (requiredSize > bufferSize) {
            bufferSize = requiredSize;
            buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);
          }

          GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, buffer.get(), &bufferSize, sizeof(RAWINPUTHEADER));

          if (auto const* const raw = reinterpret_cast<RAWINPUT*>(buffer.get()); raw->header.dwType == RIM_TYPEMOUSE) {
            if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE) {
              instance->mMouseDelta.x += raw->data.mouse.lLastX;
              instance->mMouseDelta.y += raw->data.mouse.lLastY;
            }
          }

          DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        return 0;
      }

      case WM_MOUSEMOVE: {
        if (instance->mInFocus && instance->mLockedCursorPos) {
          SetCursorPos(instance->mLockedCursorPos->x, instance->mLockedCursorPos->y);
        }

        SetCursor(instance->mHideCursor
                    ? nullptr
                    : DEFAULT_CURSOR);
      }
    }
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}


auto Window::ApplyClientAreaSize() noexcept -> void {
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


auto Window::StartUp() -> void {
  WNDCLASSEXW const wx
  {
    .cbSize = sizeof(WNDCLASSEXW),
    .lpfnWndProc = &WindowProc,
    .hInstance = GetModuleHandleW(nullptr),
    .lpszClassName = WND_CLASS_NAME
  };

  if (!RegisterClassExW(&wx)) {
    throw std::runtime_error{"Failed to register window class."};
  }

  mHwnd = CreateWindowExW(0, wx.lpszClassName, Utf8ToWide(mTitle).c_str(), WND_BORDLERLESS_STYLE, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, wx.hInstance, nullptr);

  if (!mHwnd) {
    throw std::runtime_error{"Failed to create window."};
  }

  SetWindowLongPtrW(mHwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

  RAWINPUTDEVICE const rid
  {
    .usUsagePage = HID_USAGE_PAGE_GENERIC,
    .usUsage = HID_USAGE_GENERIC_MOUSE,
    .dwFlags = 0,
    .hwndTarget = mHwnd
  };

  if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
    throw std::runtime_error{"Failed to register raw input devices."};
  }

  ShowWindow(mHwnd, SW_SHOWNORMAL);
}


auto Window::ShutDown() noexcept -> void {
  if (mHwnd) {
    DestroyWindow(mHwnd);
    mHwnd = nullptr;
  }
}


auto Window::ProcessEvents() -> void {
  // Null out delta in case a WM_INPUT is not triggered this frame to change it
  mMouseDelta = {0, 0};

  MSG msg;
  while (PeekMessageW(&msg, mHwnd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  BYTE newState[256];

  if (!GetKeyboardState(newState)) {
    throw std::runtime_error{"Failed to get keyboard state."};
  }

  for (int i = 0; i < 256; i++) {
    if (newState[i] & 0x80) {
      if (gKeyboardState[i] == KeyState::Down) {
        gKeyboardState[i] = KeyState::Held;
      } else if (gKeyboardState[i] != KeyState::Held) {
        gKeyboardState[i] = KeyState::Down;
      }
    } else {
      if (gKeyboardState[i] == KeyState::Up) {
        gKeyboardState[i] = KeyState::Neutral;
      } else {
        gKeyboardState[i] = KeyState::Up;
      }
    }
  }
}


auto Window::GetHandle() const noexcept -> HWND {
  return mHwnd;
}


auto Window::GetCurrentClientAreaSize() const noexcept -> Extent2D<std::uint32_t> {
  RECT rect;
  GetClientRect(mHwnd, &rect);
  return {static_cast<std::uint32_t>(rect.right), static_cast<std::uint32_t>(rect.bottom)};
}


auto Window::GetWindowedClientAreaSize() const noexcept -> Extent2D<std::uint32_t> {
  return mWindowedClientAreaSize;
}


auto Window::SetWindowedClientAreaSize(Extent2D<std::uint32_t> const size) noexcept -> void {
  mWindowedClientAreaSize = size;

  if (!mBorderless) {
    ApplyClientAreaSize();
  }
}


auto Window::IsBorderless() const noexcept -> bool {
  return mBorderless;
}


auto Window::SetBorderless(bool const borderless) noexcept -> void {
  if (borderless == mBorderless) {
    return;
  }

  mBorderless = borderless;

  if (mBorderless) {
    SetWindowLongPtrW(mHwnd, GWL_STYLE, WND_BORDLERLESS_STYLE | WS_VISIBLE);
    SetWindowPos(mHwnd, nullptr, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_FRAMECHANGED);
  } else {
    SetWindowLongPtrW(mHwnd, GWL_STYLE, WND_WINDOWED_STYLE | WS_VISIBLE | (mAllowWindowedResizing ? (WS_THICKFRAME) : (~WS_THICKFRAME)));
    ApplyClientAreaSize();
  }
}


auto Window::IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool {
  return mMinimizeOnBorderlessFocusLoss;
}


auto Window::SetMinimizeOnBorderlessFocusLoss(bool const minimize) noexcept -> void {
  mMinimizeOnBorderlessFocusLoss = minimize;
}


auto Window::IsQuitSignaled() const noexcept -> bool {
  return mQuitSignaled;
}


auto Window::SetQuitSignal(bool const quit) noexcept -> void {
  mQuitSignaled = quit;
}


auto Window::IsCursorLocked() const noexcept -> bool {
  return mLockedCursorPos.has_value();
}


auto Window::LockCursor(Point2D<std::int32_t> const pos) noexcept -> void {
  mLockedCursorPos = pos;
}


auto Window::UnlockCursor() noexcept -> void {
  mLockedCursorPos.reset();
}


auto Window::IsCursorHidden() const noexcept -> bool {
  return mHideCursor;
}


auto Window::SetCursorHiding(bool const hide) noexcept -> void {
  mHideCursor = hide;
}


auto Window::SetEventHook(std::function<bool(HWND, UINT, WPARAM, LPARAM)> hook) noexcept -> void {
  mEventHook = std::move(hook);
}


auto Window::GetCursorPosition() const noexcept -> Point2D<std::int32_t> {
  POINT pos;
  GetCursorPos(&pos);
  return {pos.x, pos.y};
}


auto Window::GetMouseDelta() const noexcept -> Point2D<std::int32_t> {
  return mMouseDelta;
}


auto Window::IsIgnoringManagedRequests() const noexcept -> bool {
  return mIgnoreManagedRequests;
}


auto Window::SetIgnoreManagedRequests(bool const ignore) noexcept -> void {
  mIgnoreManagedRequests = ignore;
}


auto Window::ScreenCoordinateToClient(Point2D<std::int32_t> const screenCoord) const noexcept -> Point2D<std::int32_t> {
  POINT p{screenCoord.x, screenCoord.y};
  ScreenToClient(mHwnd, &p);
  return {p.x, p.y};
}


auto Window::ClientCoordinateToScreen(Point2D<std::int32_t> const clientCoord) const noexcept -> Point2D<std::int32_t> {
  POINT p{clientCoord.x, clientCoord.y};
  ClientToScreen(mHwnd, &p);
  return {p.x, p.y};
}


auto Window::GetTitle() const noexcept -> std::string_view {
  return mTitle;
}


auto Window::SetTitle(std::string title) -> void {
  mTitle = std::move(title);
  SetWindowTextW(mHwnd, Utf8ToWide(mTitle).c_str());
}


auto Window::IsWindowedResizingAllowed() const noexcept -> bool {
  return mAllowWindowedResizing;
}


auto Window::SetWindowedResizingAllowed(bool const allowed) noexcept -> void {
  mAllowWindowedResizing = allowed;

  if (!IsBorderless()) {
    SetBorderless(false);
  }
}


auto GetKey(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Down || gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Held;
}


auto GetKeyDown(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Down;
}


auto GetKeyUp(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Up;
}


auto WideToUtf8(std::wstring_view const wstr) -> std::string {
  std::string ret(static_cast<std::size_t>(WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr)), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), ret.data(), static_cast<int>(ret.size()), nullptr, nullptr);
  return ret;
}


auto Utf8ToWide(std::string_view const str) -> std::wstring {
  std::wstring ret(static_cast<std::size_t>(MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0)), wchar_t{0});
  MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), ret.data(), static_cast<int>(ret.size()));
  return ret;
}


auto GetExecutablePath() noexcept -> std::wstring_view {
  wchar_t* exePath;
  _get_wpgmptr(&exePath);
  return exePath;
}


auto DisplayError(std::string_view const msg) noexcept -> void {
  MessageBoxA(nullptr, msg.data(), "Error", MB_ICONERROR);
}


auto DisplayError(std::wstring_view const msg) noexcept -> void {
  MessageBoxW(nullptr, msg.data(), L"Error", MB_ICONERROR);
}
}
