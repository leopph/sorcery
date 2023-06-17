#include "ImGuiIntegration.hpp"

extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;


namespace sorcery::mage {
auto EditorImGuiEventHook(HWND const hwnd, UINT const msg, WPARAM const wparam, LPARAM const lparam) -> bool {
  return ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
}
}
