#pragma once

#include <imgui.h>

using HWND = struct HWND__*;
using UINT = unsigned;
using WPARAM = unsigned long long;
using LPARAM = long long;

struct ID3D11Device;
struct ID3D11DeviceContext;


namespace sorcery::mage {
auto EditorImGuiEventHook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> bool;

IMGUI_IMPL_API bool ImGui_ImplDX11_Init(ID3D11Device* device, ID3D11DeviceContext* device_context);
IMGUI_IMPL_API void ImGui_ImplDX11_Shutdown();
IMGUI_IMPL_API void ImGui_ImplDX11_NewFrame();
IMGUI_IMPL_API void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing Dear ImGui state.
IMGUI_IMPL_API void ImGui_ImplDX11_InvalidateDeviceObjects();
IMGUI_IMPL_API bool ImGui_ImplDX11_CreateDeviceObjects();
}
