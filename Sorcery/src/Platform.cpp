#include "Platform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
// Has to be after Windows.h
#include <hidusage.h>


namespace sorcery {
namespace {
enum class KeyState : std::uint8_t {
  Neutral = 0,
  Down    = 1,
  Held    = 2,
  Up      = 3
};


KeyState gKeyboardState[256]{};
Point2D gMouseDelta{0, 0};
auto mQuitSignaled{false};
}


auto ProcessEvents() -> void {
  // Null out delta in case a WM_INPUT is not triggered this frame to change it
  gMouseDelta = {0, 0};

  MSG msg;
  while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_QUIT) {
      mQuitSignaled = true;
    }

    TranslateMessage(&msg);
    DispatchMessageW(&msg);
  }

  BYTE newState[256];

  if (!GetKeyboardState(newState)) {
    throw std::runtime_error{"Failed to get keyboard state."};
  }

  for (auto i = 0; i < 256; i++) {
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


auto IsQuitSignaled() noexcept -> bool {
  return mQuitSignaled;
}


auto GetKey(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Down || gKeyboardState[static_cast<std::uint8_t>(
           key)] == KeyState::Held;
}


auto GetKeyDown(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Down;
}


auto GetKeyUp(Key const key) noexcept -> bool {
  return gKeyboardState[static_cast<std::uint8_t>(key)] == KeyState::Up;
}


auto GetCursorPosition() noexcept -> Point2D<int> {
  POINT pos;
  GetCursorPos(&pos);
  return {pos.x, pos.y};
}


auto GetMouseDelta() noexcept -> Point2D<int> {
  return gMouseDelta;
}


auto SetMouseDelta(Point2D<int> const mouseDelta) noexcept -> void {
  gMouseDelta = mouseDelta;
}


auto WideToUtf8(std::wstring_view const wstr) -> std::string {
  std::string ret(static_cast<std::size_t>(WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()),
    nullptr, 0, nullptr, nullptr)), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), ret.data(), static_cast<int>(ret.size()),
    nullptr, nullptr);
  return ret;
}


auto Utf8ToWide(std::string_view const str) -> std::wstring {
  std::wstring ret(
    static_cast<std::size_t>(MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0)),
    wchar_t{0});
  MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), ret.data(), static_cast<int>(ret.size()));
  return ret;
}


auto GetExecutablePath() noexcept -> std::wstring_view {
#ifdef NDEBUG
    wchar_t* exePath;
  _get_wpgmptr(&exePath);
  return exePath;
#else
  static std::wstring const exe_path{
    [] {
      constexpr DWORD buf_size{10'000};
      std::wstring ret(buf_size, L'\0');
      auto const real_length{GetModuleFileNameW(nullptr, ret.data(), buf_size)};
      ret.resize(real_length);
      return ret;
    }()
  };
  return exe_path;
#endif
}


auto DisplayError(std::string_view const msg) noexcept -> void {
  MessageBoxA(nullptr, msg.data(), "Error", MB_ICONERROR);
}


auto DisplayError(std::wstring_view const msg) noexcept -> void {
  MessageBoxW(nullptr, msg.data(), L"Error", MB_ICONERROR);
}
}
