#include "SceneViewWindow.hpp"

#include "StandaloneCamera.hpp"
#include "Platform.hpp"
#include "Timing.hpp"
#include "Window.hpp"
#include "engine_context.hpp"


namespace sorcery::mage {
SceneViewWindow::SceneViewWindow() {
  g_engine_context.scene_renderer->Register(cam_);
}


SceneViewWindow::~SceneViewWindow() {
  g_engine_context.scene_renderer->Unregister(cam_);
}


auto SceneViewWindow::Draw(Application& context) -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{480, 270}, ImVec2{
    std::numeric_limits<float>::max(), std::numeric_limits<float>::max()
  });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
    ImGui::PopStyleVar();

    auto openCamSettings{false};

    if (ImGui::BeginMenuBar()) {
      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoOpCombo", gizmo_op_options_[gizmo_op_idx_].label)) {
        for (auto i{0}; i < std::ssize(gizmo_op_options_); i++) {
          if (ImGui::Selectable(gizmo_op_options_[i].label, gizmo_op_idx_ == i)) {
            gizmo_op_idx_ = i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoModeCombo", gizmo_mode_options_[gizmo_mode_idx_].label)) {
        for (auto i{0}; i < std::ssize(gizmo_mode_options_); i++) {
          if (ImGui::Selectable(gizmo_mode_options_[i].label, gizmo_mode_idx_ == i)) {
            gizmo_mode_idx_ = i;
          }
        }
        ImGui::EndCombo();
      }

      if (ImGui::Button("Camera")) {
        openCamSettings = true;
      }

      ImGui::EndMenuBar();
    }

    auto constexpr camSettingsPopupid{"camSettings"};

    if (openCamSettings) {
      ImGui::OpenPopup(camSettingsPopupid);
    }

    if (ImGui::BeginPopup(camSettingsPopupid)) {
      auto constexpr clipPlaneMin{0.01f};
      auto constexpr clipPlaneMax{10'000.0f};
      auto constexpr clipPlaneSliderSpeed{0.01f};
      auto constexpr clipPlaneSliderFormat{"%.2f"};
      auto constexpr clipPlaneSliderFlags{ImGuiSliderFlags_AlwaysClamp};

      if (auto nearPlane{cam_.GetNearClipPlane()}; ImGui::DragFloat("Near Clip Plane", &nearPlane, clipPlaneSliderSpeed,
        clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat, clipPlaneSliderFlags)) {
        cam_.SetNearClipPlane(nearPlane);
        cam_.SetFarClipPlane(std::max(nearPlane, cam_.GetFarClipPlane()));
      }

      if (auto farPlane{cam_.GetFarClipPlane()}; ImGui::DragFloat("Far Clip Plane", &farPlane, clipPlaneSliderSpeed,
        clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat, clipPlaneSliderFlags)) {
        cam_.SetFarClipPlane(farPlane);
        cam_.SetNearClipPlane(std::min(farPlane, cam_.GetNearClipPlane()));
      }

      ImGui::DragFloat("Speed", &cam_.speed, 0.1f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

      if (auto fov{cam_.GetVerticalPerspectiveFov()}; ImGui::SliderFloat("FOV", &fov, 5, 120, "%.0f",
        ImGuiSliderFlags_AlwaysClamp)) {
        cam_.SetVerticalPerspectiveFov(fov);
      }

      ImGui::EndPopup();
    }

    auto const contentRegionSize{ImGui::GetContentRegionAvail()};

    if (!cam_.GetRenderTarget() || cam_.GetRenderTarget()->GetDesc().width != static_cast<UINT>(contentRegionSize.x) ||
        cam_.GetRenderTarget()->GetDesc().height != static_cast<UINT>(contentRegionSize.y)) {
      cam_.SetRenderTarget(rendering::RenderTarget::New(*g_engine_context.graphics_device,
        rendering::RenderTarget::Desc{
          static_cast<UINT>(contentRegionSize.x), static_cast<UINT>(contentRegionSize.y), DXGI_FORMAT_R8G8B8A8_UNORM,
          std::nullopt, 1, L"Scene View RT"
        }));
    }

    auto const wasCamMoving{cam_moving_};
    cam_moving_ = wasCamMoving
                    ? ImGui::IsMouseDown(ImGuiMouseButton_Right)
                    : ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (!wasCamMoving && cam_moving_) {
      g_engine_context.window->SetCursorLock(GetCursorPosition());
      g_engine_context.window->SetCursorHiding(true);
      ImGuizmo::Enable(false);
    } else if (wasCamMoving && !cam_moving_) {
      g_engine_context.window->SetCursorLock(std::nullopt);
      g_engine_context.window->SetCursorHiding(false);
      ImGuizmo::Enable(true);
    }

    if (cam_moving_) {
      ImGui::SetWindowFocus();

      Vector3 posDelta{0, 0, 0};
      if (GetKey(Key::W) || GetKey(Key::UpArrow)) {
        posDelta += Vector3::Forward();
      }
      if (GetKey(Key::A) || GetKey(Key::LeftArrow)) {
        posDelta += Vector3::Left();
      }
      if (GetKey(Key::D) || GetKey(Key::RightArrow)) {
        posDelta += Vector3::Right();
      }
      if (GetKey(Key::S) || GetKey(Key::DownArrow)) {
        posDelta += Vector3::Backward();
      }

      Normalize(posDelta);

      if (GetKey(Key::Shift)) {
        posDelta *= 2;
      }

      cam_.position += cam_.orientation.Rotate(posDelta) * cam_.speed * timing::GetFrameTime();

      auto const [mouseX, mouseY]{GetMouseDelta()};
      auto constexpr sens{0.05f};

      cam_.orientation = Quaternion{Vector3::Up(), static_cast<f32>(mouseX) * sens} * cam_.orientation;
      cam_.orientation *= Quaternion{Vector3::Right(), static_cast<f32>(mouseY) * sens};
    }

    if (auto const selectedObject{context.GetSelectedObject()}) {
      selectedObject->OnDrawGizmosSelected();
    }

    ImGui::Image(cam_.GetRenderTarget()->GetColorTex().get(), contentRegionSize);

    auto const aspect{ImGui::GetWindowWidth() / ImGui::GetWindowHeight()};
    auto const camViewMtx{cam_.CalculateViewMatrix()};
    auto const camProjMtx{cam_.CalculateProjectionMatrix(aspect)};

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(),
      ImGui::GetWindowHeight());
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();

    if (ImGui::IsWindowFocused() && GetKeyDown(Key::G)) {
      show_grid_ = !show_grid_;
    }

    if (show_grid_) {
      ImGuizmo::DrawGrid(camViewMtx.GetData(), camProjMtx.GetData(), Matrix4::Identity().GetData(),
        cam_.GetFarClipPlane());
    }

    if (focus_target_) {
      focus_target_->t += timing::GetFrameTime() * 5.0f;
      focus_target_->t = std::min(focus_target_->t, 1.0f);
      cam_.position = Lerp(focus_target_->source, focus_target_->target, focus_target_->t);

      if (focus_target_->t >= 1.0f) {
        focus_target_.reset();
      }
    }

    if (!context.GetImGuiIo().WantTextInput && !cam_moving_) {
      if (GetKeyDown(Key::Q)) {
        gizmo_op_idx_ = 0;
      }
      if (GetKeyDown(Key::W)) {
        gizmo_op_idx_ = 1;
      }
      if (GetKeyDown(Key::E)) {
        gizmo_op_idx_ = 2;
      }
      if (GetKeyDown(Key::R)) {
        gizmo_mode_idx_ = 1 - gizmo_mode_idx_;
      }
    }

    if (auto const selectedEntity{dynamic_cast<Entity*>(context.GetSelectedObject())}; selectedEntity) {
      if (!context.GetImGuiIo().WantTextInput && !cam_moving_ && GetKeyDown(Key::F)) {
        focus_target_.emplace(cam_.GetPosition(),
          selectedEntity->GetTransform().GetWorldPosition() - cam_.GetForwardAxis() * 2, 0.0f);
      }

      // TODO this breaks if the transform is a child of a rotated transform
      if (Matrix4 modelMat{selectedEntity->GetTransform().GetLocalToWorldMatrix()}; Manipulate(camViewMtx.GetData(),
        camProjMtx.GetData(), gizmo_op_options_[gizmo_op_idx_].op, gizmo_mode_options_[gizmo_mode_idx_].mode,
        modelMat.GetData())) {
        Vector3 pos, euler, scale;
        ImGuizmo::DecomposeMatrixToComponents(modelMat.GetData(), pos.GetData(), euler.GetData(), scale.GetData());
        selectedEntity->GetTransform().SetWorldPosition(pos);
        selectedEntity->GetTransform().SetWorldRotation(Quaternion::FromEulerAngles(euler));
        selectedEntity->GetTransform().SetWorldScale(scale);
      }
    }
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}


auto SceneViewWindow::GetCamera() noexcept -> StandaloneCamera& {
  return cam_;
}
}
