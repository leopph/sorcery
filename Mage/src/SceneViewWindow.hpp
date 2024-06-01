#pragma once

#include "editor_gui.hpp"
#include "StandaloneCamera.hpp"

#include <array>
#include <optional>


namespace sorcery::mage {
class EditorApp;


class SceneViewWindow {
public:
  SceneViewWindow();
  SceneViewWindow(SceneViewWindow const&) = delete;
  SceneViewWindow(SceneViewWindow&&) = delete;

  ~SceneViewWindow();

  auto operator=(SceneViewWindow const&) -> void = delete;
  auto operator=(SceneViewWindow&&) -> void = delete;

  auto Draw(EditorApp& context) -> void;
  [[nodiscard]] auto GetCamera() noexcept -> StandaloneCamera&;

private:
  struct FocusMoveInfo {
    Vector3 source;
    Vector3 target;
    float t;
  };


  struct GizmoOpOption {
    ImGuizmo::OPERATION op;
    char const* label;
  };


  struct GizmoModeOption {
    ImGuizmo::MODE mode;
    char const* label;
  };


  static std::array constexpr gizmo_op_options_{
    GizmoOpOption{ImGuizmo::OPERATION::TRANSLATE, "Translate"}, GizmoOpOption{ImGuizmo::OPERATION::ROTATE, "Rotate"},
    GizmoOpOption{ImGuizmo::OPERATION::SCALE, "Scale"}
  };

  static std::array constexpr gizmo_mode_options_{
    GizmoModeOption{ImGuizmo::MODE::LOCAL, "Local"}, GizmoModeOption{ImGuizmo::MODE::WORLD, "World"}
  };

  StandaloneCamera cam_{Vector3{}, Quaternion{}, 5.0f, 0.1f, 1000.0f, 60};
  std::optional<FocusMoveInfo> focus_target_;
  int gizmo_op_idx_{0};
  int gizmo_mode_idx_{0};
  bool cam_moving_{false};
  bool show_grid_{false};
};
}
