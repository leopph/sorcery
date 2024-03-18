#pragma once

#include "Core.hpp"
#include "Event.hpp"
#include "Util.hpp"


namespace sorcery {
class WindowImpl;


class Window {
  WindowImpl* mImpl;

public:
  LEOPPHAPI explicit Window();
  Window(Window const&) = delete;
  Window(Window&&) = delete;

  LEOPPHAPI ~Window();

  auto operator=(Window const&) -> void = delete;
  auto operator=(Window&&) -> void = delete;

  GuardedEventReference<Extent2D<unsigned>>& OnWindowSize;
  GuardedEventReference<>& OnWindowFocusGain;
  GuardedEventReference<>& OnWindowFocusLoss;

  [[nodiscard]] LEOPPHAPI auto GetNativeHandle() const noexcept -> void*;

  [[nodiscard]] LEOPPHAPI auto GetSize() const noexcept -> Extent2D<int>;
  LEOPPHAPI auto SetSize(Extent2D<int> size) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetClientAreaSize() const noexcept -> Extent2D<int>;
  LEOPPHAPI auto SetClientAreaSize(Extent2D<int> size) noexcept -> void;

  // Screen space is [0, 1] from left to right, bottom to top.
  LEOPPHAPI auto SetPosition(Point2D<int> position) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsBorderless() const noexcept -> bool;
  LEOPPHAPI auto SetBorderless(bool borderless) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsMinimizingOnBorderlessFocusLoss() const noexcept -> bool;
  LEOPPHAPI auto SetMinimizeOnBorderlessFocusLoss(bool minimize) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsCursorHidden() const noexcept -> bool;
  LEOPPHAPI auto SetCursorHiding(bool hide) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto IsCursorLocked() const noexcept -> bool;
  LEOPPHAPI auto SetCursorLock(std::optional<Point2D<int>> pos) noexcept -> void;

  [[nodiscard]] LEOPPHAPI auto GetTitle() const noexcept -> std::string_view;
  LEOPPHAPI auto SetTitle(std::string_view title) -> void;

  [[nodiscard]] LEOPPHAPI auto ScreenCoordinateToClient(Point2D<int> screenCoord) const noexcept -> Point2D<int>;
  [[nodiscard]] LEOPPHAPI auto ClientCoordinateToScreen(Point2D<int> clientCoord) const noexcept -> Point2D<int>;

  LEOPPHAPI auto SetEventHandler(void const* handler) noexcept -> void;

  LEOPPHAPI auto UseImmersiveDarkMode(bool value) noexcept -> void;
};
}
