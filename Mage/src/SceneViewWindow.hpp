#pragma once

#include "Application.hpp"
#include "StandaloneCamera.hpp"
#include "RenderTarget.hpp"

#include <ImGuizmo.h>

#include <array>
#include <optional>


namespace sorcery::mage {
class SceneViewWindow {
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


  static std::array constexpr GIZMO_OP_OPTIONS{
    GizmoOpOption{ImGuizmo::OPERATION::TRANSLATE, "Translate"},
    GizmoOpOption{ImGuizmo::OPERATION::ROTATE, "Rotate"},
    GizmoOpOption{ImGuizmo::OPERATION::SCALE, "Scale"}
  };

  static std::array constexpr GIZMO_MODE_OPTIONS{
    GizmoModeOption{ImGuizmo::MODE::LOCAL, "Local"},
    GizmoModeOption{ImGuizmo::MODE::WORLD, "World"}
  };


  std::unique_ptr<RenderTarget> mRenderTarget;
  StandaloneCamera mCam{Vector3{}, Quaternion{}, 5.0f, 0.1f, 1000.0f, 60};
  std::optional<FocusMoveInfo> mFocusTarget;

  int mGizmoOpIdx{0};
  int mGizmoModeIdx{0};
  bool mCamMoving{false};
  bool mShowGrid{false};

public:
  auto Draw(Application& context) -> void;
  [[nodiscard]] auto GetCamera() noexcept -> StandaloneCamera&;
};
}
