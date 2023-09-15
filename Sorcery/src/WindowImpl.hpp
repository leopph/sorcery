#pragma once

#include "Window.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>


namespace sorcery {
class WindowImpl {
  using WindowProcType = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);

  HWND mHwnd{nullptr};
  HCURSOR mCursor{LoadCursorW(nullptr, IDC_ARROW)};
  WindowProcType mEventHandler{nullptr};
  bool mIsBorderless{false};
  bool mMinimizeOnBorderlessFocusLoss{false};
  bool mHideCursor{false};
  bool mIsInFocus{true};
  std::string mTitle{"Sorcery"};
  std::optional<Point2D<int>> mLockedCursorPos;

  Event<Extent2D<unsigned>> mOnSizeEvent;
  Event<> mOnFocusGainEvent;
  Event<> mOnFocusLossEvent;

  static auto CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept -> LRESULT;
  [[nodiscard]] static auto RectToExtent(RECT rect) noexcept -> Extent2D<int>;

public:
  auto StartUp() -> void;
  auto ShutDown() noexcept -> void;

  GuardedEventReference<Extent2D<std::uint32_t>> OnWindowSize{mOnSizeEvent};
  GuardedEventReference<> OnWindowFocusGain{mOnFocusGainEvent};
  GuardedEventReference<> OnWindowFocusLoss{mOnFocusLossEvent};

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
};
}
