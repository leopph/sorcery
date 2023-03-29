#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace leopph::editor {
auto EditorImGuiEventHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> bool;
}
