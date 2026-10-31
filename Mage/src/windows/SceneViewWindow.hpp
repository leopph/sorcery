#pragma once

#include <array>
#include <optional>

#include <imgui.h>
#include <ImGuizmo.h>

#include "../StandaloneCamera.hpp"
#include "sorcery/observer_ptr.hpp"
#include "sorcery/host/rendering/view.hpp"


namespace sorcery {
class World;


namespace mage {
class EditorApp;
class ImGuiTextureReferences;


class SceneViewWindow {
public:
  SceneViewWindow(ImGuiTextureReferences& tex_refs, World& edit_world);
  SceneViewWindow(SceneViewWindow const&) = delete;
  SceneViewWindow(SceneViewWindow&&) = delete;

  ~SceneViewWindow() = default;

  auto operator=(SceneViewWindow const&) -> void = delete;
  auto operator=(SceneViewWindow&&) -> void = delete;

  auto Draw(EditorApp& context) -> void;
  auto AddViews(rendering::ViewList& views) const -> void;
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

  ObserverPtr<ImGuiTextureReferences> tex_refs_;
  ObserverPtr<World> edit_world_;
  StandaloneCamera cam_{Vector3{}, Quaternion{}, 5.0f, 0.1f, 1000.0f, 60};
  std::shared_ptr<rendering::RenderTarget> rt_;
  rendering::ViewHistoryKey view_history_{rendering::ViewHistoryKey::MakeUnique()};
  std::optional<FocusMoveInfo> focus_target_;
  int gizmo_op_idx_{0};
  int gizmo_mode_idx_{0};
  bool cam_moving_{false};
  bool show_grid_{false};
  bool was_visible_{false};
};
}
}
