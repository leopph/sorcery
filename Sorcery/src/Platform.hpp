#pragma once

#include "Core.hpp"
#include "point.hpp"

#include <string>


namespace sorcery {
enum class Key : int;

LEOPPHAPI auto ProcessEvents() -> void;
LEOPPHAPI auto IsQuitSignaled() noexcept -> bool;

[[nodiscard]] LEOPPHAPI auto GetKey(Key key) noexcept -> bool;
[[nodiscard]] LEOPPHAPI auto GetKeyDown(Key key) noexcept -> bool;
[[nodiscard]] LEOPPHAPI auto GetKeyUp(Key key) noexcept -> bool;

[[nodiscard]] LEOPPHAPI auto GetCursorPosition() noexcept -> Point2D<int>;
[[nodiscard]] LEOPPHAPI auto GetMouseDelta() noexcept -> Point2D<int>;
LEOPPHAPI auto SetMouseDelta(Point2D<int> mouseDelta) noexcept -> void;

[[nodiscard]] LEOPPHAPI auto WideToUtf8(std::wstring_view wstr) -> std::string;
[[nodiscard]] LEOPPHAPI auto Utf8ToWide(std::string_view str) -> std::wstring;

[[nodiscard]] LEOPPHAPI auto GetExecutablePath() noexcept -> std::wstring_view;

LEOPPHAPI auto DisplayError(std::string_view msg) noexcept -> void;
LEOPPHAPI auto DisplayError(std::wstring_view msg) noexcept -> void;


enum class Key : int {
  LeftMouseButton   = 0x01,
  RightMouseButton  = 0x02,
  Cancel            = 0x03,
  MiddleMouseButton = 0x04,
  XButton1          = 0x05,
  XButton2          = 0x06,
  Backspace         = 0x08,
  Tab               = 0x09,
  Clear             = 0x0C,
  Enter             = 0x0D,
  Shift             = 0x10,
  Control           = 0x11,
  Alt               = 0x12,
  Pause             = 0x13,
  CapsLock          = 0x14,
  Escape            = 0x1B,
  Space             = 0x20,
  PageUp            = 0x21,
  PageDown          = 0x22,
  End               = 0x23,
  Home              = 0x24,
  LeftArrow         = 0x25,
  UpArrow           = 0x26,
  RightArrow        = 0x27,
  DownArrow         = 0x28,
  Select            = 0x29,
  Print             = 0x2A,
  Execute           = 0x2B,
  PrintScreen       = 0x2C,
  Insert            = 0x2D,
  Delete            = 0x2E,
  Help              = 0x2F,
  Zero              = 0x30,
  One               = 0x31,
  Two               = 0x32,
  Three             = 0x33,
  Four              = 0x34,
  Five              = 0x35,
  Six               = 0x36,
  Seven             = 0x37,
  Eight             = 0x38,
  Nine              = 0x39,
  A                 = 0x41,
  B                 = 0x42,
  C                 = 0x43,
  D                 = 0x44,
  E                 = 0x45,
  F                 = 0x46,
  G                 = 0x47,
  H                 = 0x48,
  I                 = 0x49,
  J                 = 0x4A,
  K                 = 0x4B,
  L                 = 0x4C,
  M                 = 0x4D,
  N                 = 0x4E,
  O                 = 0x4F,
  P                 = 0x50,
  Q                 = 0x51,
  R                 = 0x52,
  S                 = 0x53,
  T                 = 0x54,
  U                 = 0x55,
  V                 = 0x56,
  W                 = 0x57,
  X                 = 0x58,
  Y                 = 0x59,
  Z                 = 0x5A,
  LeftWindows       = 0x5B,
  RightWindow       = 0x5C,
  Apps              = 0x5D,
  Sleep             = 0x5F,
  NumPad0           = 0x60,
  NumPad1           = 0x61,
  NumPad2           = 0x62,
  NumPad3           = 0x63,
  NumPad4           = 0x64,
  NumPad5           = 0x65,
  NumPad6           = 0x66,
  NumPad7           = 0x67,
  NumPad8           = 0x68,
  NumPad9           = 0x69,
  Multiply          = 0x6A,
  Add               = 0x6B,
  Separator         = 0x6C,
  Subtract          = 0x6D,
  Decimal           = 0x6E,
  Divide            = 0x6F,
  F1                = 0x70,
  F2                = 0x71,
  F3                = 0x72,
  F4                = 0x73,
  F5                = 0x74,
  F6                = 0x75,
  F7                = 0x76,
  F8                = 0x77,
  F9                = 0x78,
  F10               = 0x79,
  F11               = 0x7A,
  F12               = 0x7B,
  F13               = 0x7C,
  F14               = 0x7D,
  F15               = 0x7E,
  F16               = 0x7F,
  F17               = 0x80,
  F18               = 0x81,
  F19               = 0x82,
  F20               = 0x83,
  F21               = 0x84,
  F22               = 0x85,
  F23               = 0x86,
  F24               = 0x87,
  NumLock           = 0x90,
  ScrollLock        = 0x91,
  LeftShift         = 0xA0,
  RightShift        = 0xA1,
  LeftControl       = 0xA2,
  RightControl      = 0xA3,
  LeftAlt           = 0xA4,
  RightAlt          = 0xA5,
};
}
