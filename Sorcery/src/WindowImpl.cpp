#include "WindowImpl.hpp"

#include "Platform.hpp"

#include <hidusage.h>


namespace sorcery {
auto CALLBACK WindowImpl::WindowProc(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) noexcept -> LRESULT {
  if (auto const self{reinterpret_cast<WindowImpl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))}) {
    if (self->mEventHandler && self->mEventHandler(hwnd, msg, wparam, lparam)) {
      return true;
    }

    switch (msg) {
      case WM_CLOSE: {
        PostQuitMessage(0);
        return 0;
      }

      case WM_SIZE: {
        self->mOnSizeEvent.invoke({LOWORD(lparam), HIWORD(lparam)});
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
          if (self->mIsBorderless && self->mMinimizeOnBorderlessFocusLoss) {
            ShowWindow(hwnd, SW_MINIMIZE);
          }
          self->mIsInFocus = false;
          self->mOnFocusLossEvent.invoke();
        } else {
          self->mIsInFocus = true;
          self->mOnFocusGainEvent.invoke();
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
              auto mouseDelta{GetMouseDelta()};
              mouseDelta.x += raw->data.mouse.lLastX;
              mouseDelta.y += raw->data.mouse.lLastY;
              SetMouseDelta(mouseDelta);
            }
          }

          DefWindowProcW(hwnd, msg, wparam, lparam);
        }

        return 0;
      }

      case WM_MOUSEMOVE: {
        if (self->mIsInFocus && self->mLockedCursorPos) {
          SetCursorPos(self->mLockedCursorPos->x, self->mLockedCursorPos->y);
        }

        SetCursor(self->mHideCursor ? nullptr : self->mCursor);
      }
    }
  }

  return DefWindowProcW(hwnd, msg, wparam, lparam);
}


auto WindowImpl::RectToExtent(RECT const rect) noexcept -> Extent2D<int> {
  int const width{rect.right - rect.left};
  int const height{rect.bottom - rect.top};
  return Extent2D{.width = width, .height = height};
}


auto WindowImpl::StartUp() -> void {
  WNDCLASSEXW const wx{
    .cbSize = sizeof(WNDCLASSEXW),
    .lpfnWndProc = &WindowProc,
    .hInstance = GetModuleHandleW(nullptr),
    .lpszClassName = L"SorceryWindowClass"
  };

  if (!RegisterClassExW(&wx)) {
    throw std::runtime_error{"Failed to register window class."};
  }

  mHwnd = CreateWindowExW(0, wx.lpszClassName, Utf8ToWide(mTitle).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wx.hInstance, nullptr);

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


auto WindowImpl::ShutDown() noexcept -> void {
  if (mHwnd) {
    DestroyWindow(mHwnd);
    mHwnd = nullptr;
  }
}


auto WindowImpl::GetNativeHandle() const noexcept -> void* {
  return mHwnd;
}


auto WindowImpl::GetSize() const noexcept -> Extent2D<int> {
  RECT rect;
  GetWindowRect(mHwnd, &rect);
  return RectToExtent(rect);
}


auto WindowImpl::SetSize(Extent2D<int> const size) noexcept -> void {
  SetWindowPos(mHwnd, nullptr, 0, 0, size.width, size.height, SWP_NOMOVE);
}


auto WindowImpl::GetClientAreaSize() const noexcept -> Extent2D<int> {
  RECT rect;
  GetClientRect(mHwnd, &rect);
  return RectToExtent(rect);
}


auto WindowImpl::SetClientAreaSize(Extent2D<int> const size) noexcept -> void {
  RECT rect{
    .left = 0,
    .top = 0,
    .right = size.width,
    .bottom = size.height
  };

  AdjustWindowRect(&rect, static_cast<DWORD>(GetWindowLongPtrW(mHwnd, GWL_STYLE)), FALSE);
  SetWindowPos(mHwnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
}


auto WindowImpl::SetPosition(Point2D<int> position) noexcept -> void {
  SetWindowPos(mHwnd, nullptr, position.x, -position.y, 0, 0, SWP_NOSIZE);
}


auto WindowImpl::IsBorderless() const noexcept -> bool {
  return mIsBorderless;
}


auto WindowImpl::SetBorderless(bool const borderless) noexcept -> void {
  if (borderless == mIsBorderless) {
    return;
  }

  SetWindowLongPtrW(mHwnd, GWL_STYLE, (borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
  SetWindowPos(mHwnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);

  mIsBorderless = borderless;
}


auto WindowImpl::IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool {
  return mMinimizeOnBorderlessFocusLoss;
}


auto WindowImpl::SetMinimizeOnBorderlessFocusLoss(bool const minimize) noexcept -> void {
  mMinimizeOnBorderlessFocusLoss = minimize;
}


auto WindowImpl::IsCursorHidden() const noexcept -> bool {
  return mHideCursor;
}


auto WindowImpl::SetCursorHiding(bool const hide) noexcept -> void {
  mHideCursor = hide;
}


auto WindowImpl::IsCursorLocked() const noexcept -> bool {
  return mLockedCursorPos.has_value();
}


auto WindowImpl::SetCursorLock(std::optional<Point2D<int>> const pos) noexcept -> void {
  mLockedCursorPos = pos;
}


auto WindowImpl::GetTitle() const noexcept -> std::string_view {
  return mTitle;
}


auto WindowImpl::SetTitle(std::string_view const title) -> void {
  mTitle = title;
  SetWindowTextW(mHwnd, Utf8ToWide(title).c_str());
}


auto WindowImpl::ScreenCoordinateToClient(Point2D<int> const screenCoord) const noexcept -> Point2D<int> {
  POINT p{screenCoord.x, screenCoord.y};
  ScreenToClient(mHwnd, &p);
  return {p.x, p.y};
}


auto WindowImpl::ClientCoordinateToScreen(Point2D<int> const clientCoord) const noexcept -> Point2D<int> {
  POINT p{clientCoord.x, clientCoord.y};
  ClientToScreen(mHwnd, &p);
  return {p.x, p.y};
}


auto WindowImpl::SetEventHandler(void const* handler) noexcept -> void {
  mEventHandler = reinterpret_cast<WindowProcType>(handler);
}
}
