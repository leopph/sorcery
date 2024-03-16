#include "WindowImpl.hpp"

#include "Platform.hpp"

#include <dwmapi.h>
#include <hidusage.h>

#include <cassert>


namespace sorcery {
WindowImpl::WindowImpl(graphics::GraphicsDevice& graphics_device) :
  graphics_device_{&graphics_device} {
  WNDCLASSEXW const wx{
    .cbSize = sizeof(WNDCLASSEXW), .lpfnWndProc = &WindowProc, .hInstance = GetModuleHandleW(nullptr),
    .lpszClassName = L"SorceryWindowClass"
  };

  if (!RegisterClassExW(&wx)) {
    throw std::runtime_error{"Failed to register window class."};
  }

  hwnd_ = CreateWindowExW(0, wx.lpszClassName, Utf8ToWide(title_).c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, wx.hInstance, nullptr);

  if (!hwnd_) {
    throw std::runtime_error{"Failed to create window."};
  }

  RAWINPUTDEVICE const rid{
    .usUsagePage = HID_USAGE_PAGE_GENERIC, .usUsage = HID_USAGE_GENERIC_MOUSE, .dwFlags = 0, .hwndTarget = hwnd_
  };

  if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE))) {
    throw std::runtime_error{"Failed to register raw input devices."};
  }

  swap_chain_ = graphics_device_->CreateSwapChain(graphics::SwapChainDesc{
    0, 0, 2, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_USAGE_RENDER_TARGET_OUTPUT, DXGI_SCALING_NONE
  }, hwnd_);

  SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
  ShowWindow(hwnd_, SW_SHOWNORMAL);
}


WindowImpl::~WindowImpl() {
  if (hwnd_) {
    DestroyWindow(hwnd_);
    hwnd_ = nullptr;
  }
}


auto WindowImpl::GetNativeHandle() const noexcept -> void* {
  return hwnd_;
}


auto WindowImpl::GetSize() const noexcept -> Extent2D<int> {
  RECT rect;
  GetWindowRect(hwnd_, &rect);
  return RectToExtent(rect);
}


auto WindowImpl::SetSize(Extent2D<int> const size) noexcept -> void {
  SetWindowPos(hwnd_, nullptr, 0, 0, size.width, size.height, SWP_NOMOVE);
}


auto WindowImpl::GetClientAreaSize() const noexcept -> Extent2D<int> {
  RECT rect;
  GetClientRect(hwnd_, &rect);
  return RectToExtent(rect);
}


auto WindowImpl::SetClientAreaSize(Extent2D<int> const size) noexcept -> void {
  RECT rect{.left = 0, .top = 0, .right = size.width, .bottom = size.height};

  AdjustWindowRect(&rect, static_cast<DWORD>(GetWindowLongPtrW(hwnd_, GWL_STYLE)), FALSE);
  SetWindowPos(hwnd_, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE);
}


auto WindowImpl::SetPosition(Point2D<int> position) noexcept -> void {
  SetWindowPos(hwnd_, nullptr, position.x, -position.y, 0, 0, SWP_NOSIZE);
}


auto WindowImpl::IsBorderless() const noexcept -> bool {
  return is_borderless_;
}


auto WindowImpl::SetBorderless(bool const borderless) noexcept -> void {
  if (borderless == is_borderless_) {
    return;
  }

  SetWindowLongPtrW(hwnd_, GWL_STYLE, (borderless ? WS_POPUP : WS_OVERLAPPEDWINDOW) | WS_VISIBLE);
  SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE);

  is_borderless_ = borderless;
}


auto WindowImpl::IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool {
  return minimize_on_borderless_focus_loss_;
}


auto WindowImpl::SetMinimizeOnBorderlessFocusLoss(bool const minimize) noexcept -> void {
  minimize_on_borderless_focus_loss_ = minimize;
}


auto WindowImpl::IsCursorHidden() const noexcept -> bool {
  return hide_cursor_;
}


auto WindowImpl::SetCursorHiding(bool const hide) noexcept -> void {
  hide_cursor_ = hide;
}


auto WindowImpl::IsCursorLocked() const noexcept -> bool {
  return locked_cursor_pos_.has_value();
}


auto WindowImpl::SetCursorLock(std::optional<Point2D<int>> const pos) noexcept -> void {
  locked_cursor_pos_ = pos;
}


auto WindowImpl::GetTitle() const noexcept -> std::string_view {
  return title_;
}


auto WindowImpl::SetTitle(std::string_view const title) -> void {
  title_ = title;
  SetWindowTextW(hwnd_, Utf8ToWide(title).c_str());
}


auto WindowImpl::ScreenCoordinateToClient(Point2D<int> const screenCoord) const noexcept -> Point2D<int> {
  POINT p{screenCoord.x, screenCoord.y};
  ScreenToClient(hwnd_, &p);
  return {p.x, p.y};
}


auto WindowImpl::ClientCoordinateToScreen(Point2D<int> const clientCoord) const noexcept -> Point2D<int> {
  POINT p{clientCoord.x, clientCoord.y};
  ClientToScreen(hwnd_, &p);
  return {p.x, p.y};
}


auto WindowImpl::SetEventHandler(void const* handler) noexcept -> void {
  handler_ = reinterpret_cast<WindowProcType>(handler);
}


auto WindowImpl::UseImmersiveDarkMode(bool const value) noexcept -> void {
  BOOL const useImmersiveDarkMode{value ? TRUE : FALSE};
  [[maybe_unused]] auto const hr{
    DwmSetWindowAttribute(hwnd_, DWMWA_USE_IMMERSIVE_DARK_MODE, &useImmersiveDarkMode, sizeof(useImmersiveDarkMode))
  };
  assert(SUCCEEDED(hr));
}


auto WindowImpl::GetSwapChain() const -> graphics::SharedDeviceChildHandle<graphics::SwapChain> const& {
  return swap_chain_;
}


auto CALLBACK WindowImpl::WindowProc(HWND const hwnd, UINT const msg, WPARAM const wparam,
                                     LPARAM const lparam) noexcept -> LRESULT {
  if (auto const self{reinterpret_cast<WindowImpl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA))}) {
    if (self->handler_ && self->handler_(hwnd, msg, wparam, lparam)) {
      return true;
    }

    switch (msg) {
      case WM_CLOSE: {
        PostQuitMessage(0);
        return 0;
      }

      case WM_SIZE: {
        self->graphics_device_->WaitIdle();
        self->graphics_device_->SwapChainResize(*self->swap_chain_, 0, 0);
        self->on_size_event_.invoke({LOWORD(lparam), HIWORD(lparam)});
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
          if (self->is_borderless_ && self->minimize_on_borderless_focus_loss_) {
            ShowWindow(hwnd, SW_MINIMIZE);
          }
          self->is_in_focus_ = false;
          self->on_focus_loss_event_.invoke();
        } else {
          self->is_in_focus_ = true;
          self->on_focus_gain_event_.invoke();
        }
        return 0;
      }

      case WM_INPUT: {
        if (wparam == RIM_INPUT) {
          UINT requiredSize;
          GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, nullptr, &requiredSize,
            sizeof(RAWINPUTHEADER));

          auto static bufferSize = requiredSize;
          auto static buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);

          if (requiredSize > bufferSize) {
            bufferSize = requiredSize;
            buffer = std::make_unique_for_overwrite<BYTE[]>(requiredSize);
          }

          GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, buffer.get(), &bufferSize,
            sizeof(RAWINPUTHEADER));

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
        if (self->is_in_focus_ && self->locked_cursor_pos_) {
          SetCursorPos(self->locked_cursor_pos_->x, self->locked_cursor_pos_->y);
        }

        SetCursor(self->hide_cursor_ ? nullptr : self->cursor_);
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
}
