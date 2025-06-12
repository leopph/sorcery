#pragma once

#include "Window.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <string>
#include <Windows.h>


namespace sorcery {
class WindowImpl {
  Event<Extent2D<unsigned>> on_size_event_;
  Event<> on_focus_gain_event_;
  Event<> on_focus_loss_event_;

public:
  explicit WindowImpl();
  WindowImpl(WindowImpl const&) = delete;
  WindowImpl(WindowImpl&&) = delete;

  ~WindowImpl();

  auto operator=(WindowImpl const&) -> void = delete;
  auto operator=(WindowImpl&&) -> void = delete;

  GuardedEventReference<Extent2D<std::uint32_t>> OnWindowSize{on_size_event_};
  GuardedEventReference<> OnWindowFocusGain{on_focus_gain_event_};
  GuardedEventReference<> OnWindowFocusLoss{on_focus_loss_event_};

  [[nodiscard]] auto GetNativeHandle() const noexcept -> void*;

  [[nodiscard]] auto GetSize() const noexcept -> Extent2D<int>;
  auto SetSize(Extent2D<int> size) noexcept -> void;

  [[nodiscard]] auto GetClientAreaSize() const noexcept -> Extent2D<int>;
  auto SetClientAreaSize(Extent2D<int> size) noexcept -> void;

  auto SetPosition(Point2D<int> position) noexcept -> void;

  [[nodiscard]] auto IsBorderless() const noexcept -> bool;
  auto SetBorderless(bool borderless) noexcept -> void;

  [[nodiscard]] auto IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool;
  auto SetMinimizeOnBorderlessFocusLoss(bool minimize) noexcept -> void;

  [[nodiscard]] auto IsCursorHidden() const noexcept -> bool;
  auto SetCursorHiding(bool hide) noexcept -> void;

  [[nodiscard]] auto IsCursorLocked() const noexcept -> bool;
  auto SetCursorLock(std::optional<Point2D<int>> pos) noexcept -> void;

  [[nodiscard]] auto GetTitle() const noexcept -> std::string_view;
  auto SetTitle(std::string_view title) -> void;

  [[nodiscard]] auto ScreenCoordinateToClient(Point2D<int> screenCoord) const noexcept -> Point2D<int>;
  [[nodiscard]] auto ClientCoordinateToScreen(Point2D<int> clientCoord) const noexcept -> Point2D<int>;

  auto SetEventHandler(void const* handler) noexcept -> void;

  auto UseImmersiveDarkMode(bool value) noexcept -> void;

private:
  using WindowProcType = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);

  static auto CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;
  [[nodiscard]] static auto RectToExtent(RECT rect) noexcept -> Extent2D<int>;

  HWND hwnd_{nullptr};
  HCURSOR cursor_{LoadCursorW(nullptr, IDC_ARROW)};
  WindowProcType handler_{nullptr};
  bool is_borderless_{false};
  bool minimize_on_borderless_focus_loss_{false};
  bool hide_cursor_{false};
  bool is_in_focus_{true};
  std::string title_{"Sorcery"};
  std::optional<Point2D<int>> locked_cursor_pos_;
};
}
