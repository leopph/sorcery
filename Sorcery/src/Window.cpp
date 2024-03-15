#include "Window.hpp"

#include "WindowImpl.hpp"

#include <cassert>


namespace sorcery {
Window::Window(graphics::GraphicsDevice& graphics_device) :
  mImpl{new WindowImpl{graphics_device}},
  OnWindowSize{mImpl->OnWindowSize},
  OnWindowFocusGain{mImpl->OnWindowFocusGain},
  OnWindowFocusLoss{mImpl->OnWindowFocusLoss} {}


Window::~Window() {
  delete mImpl;
  mImpl = nullptr;
}


auto Window::GetNativeHandle() const noexcept -> void* {
  return mImpl->GetNativeHandle();
}


auto Window::GetSize() const noexcept -> Extent2D<int> {
  assert(mImpl);
  return mImpl->GetSize();
}


auto Window::SetSize(Extent2D<int> const size) noexcept -> void {
  assert(mImpl);
  mImpl->SetSize(size);
}


auto Window::GetClientAreaSize() const noexcept -> Extent2D<int> {
  assert(mImpl);
  return mImpl->GetClientAreaSize();
}


auto Window::SetClientAreaSize(Extent2D<int> const size) noexcept -> void {
  assert(mImpl);
  mImpl->SetClientAreaSize(size);
}


auto Window::SetPosition(Point2D<int> const position) noexcept -> void {
  assert(mImpl);
  mImpl->SetPosition(position);
}


auto Window::IsBorderless() const noexcept -> bool {
  assert(mImpl);
  return mImpl->IsBorderless();
}


auto Window::SetBorderless(bool const borderless) noexcept -> void {
  assert(mImpl);
  mImpl->SetBorderless(borderless);
}


auto Window::IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool {
  assert(mImpl);
  return mImpl->IsMinimizingOnBorderlessFocusLoss();
}


auto Window::SetMinimizeOnBorderlessFocusLoss(bool const minimize) noexcept -> void {
  assert(mImpl);
  mImpl->SetMinimizeOnBorderlessFocusLoss(minimize);
}


auto Window::IsCursorHidden() const noexcept -> bool {
  assert(mImpl);
  return mImpl->IsCursorHidden();
}


auto Window::SetCursorHiding(bool const hide) noexcept -> void {
  assert(mImpl);
  mImpl->SetCursorHiding(hide);
}


auto Window::IsCursorLocked() const noexcept -> bool {
  assert(mImpl);
  return mImpl->IsCursorLocked();
}


auto Window::SetCursorLock(std::optional<Point2D<int>> const pos) noexcept -> void {
  assert(mImpl);
  mImpl->SetCursorLock(pos);
}


auto Window::GetTitle() const noexcept -> std::string_view {
  assert(mImpl);
  return mImpl->GetTitle();
}


auto Window::SetTitle(std::string_view const title) -> void {
  assert(mImpl);
  mImpl->SetTitle(title);
}


auto Window::ScreenCoordinateToClient(Point2D<int> const screenCoord) const noexcept -> Point2D<int> {
  assert(mImpl);
  return mImpl->ScreenCoordinateToClient(screenCoord);
}


auto Window::ClientCoordinateToScreen(Point2D<int> const clientCoord) const noexcept -> Point2D<int> {
  assert(mImpl);
  return mImpl->ClientCoordinateToScreen(clientCoord);
}


auto Window::SetEventHandler(void const* handler) noexcept -> void {
  assert(mImpl);
  mImpl->SetEventHandler(handler);
}


auto Window::UseImmersiveDarkMode(bool const value) noexcept -> void {
  assert(mImpl);
  mImpl->UseImmersiveDarkMode(value);
}


auto Window::GetSwapChain() const -> graphics::SharedDeviceChildHandle<graphics::SwapChain> const& {
  assert(mImpl);
  return mImpl->GetSwapChain();
}
}
