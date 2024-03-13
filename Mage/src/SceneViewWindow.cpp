#include "SceneViewWindow.hpp"

#include "StandaloneCamera.hpp"
#include "Platform.hpp"
#include "Timing.hpp"
#include "Window.hpp"
#include "engine_context.hpp"


namespace sorcery::mage {
auto SceneViewWindow::Draw(Application& context) -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{480, 270}, ImVec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
    ImGui::PopStyleVar();

    auto openCamSettings{false};

    if (ImGui::BeginMenuBar()) {
      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoOpCombo", GIZMO_OP_OPTIONS[mGizmoOpIdx].label)) {
        for (auto i{0}; i < std::ssize(GIZMO_OP_OPTIONS); i++) {
          if (ImGui::Selectable(GIZMO_OP_OPTIONS[i].label, mGizmoOpIdx == i)) {
            mGizmoOpIdx = i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoModeCombo", GIZMO_MODE_OPTIONS[mGizmoModeIdx].label)) {
        for (auto i{0}; i < std::ssize(GIZMO_MODE_OPTIONS); i++) {
          if (ImGui::Selectable(GIZMO_MODE_OPTIONS[i].label, mGizmoModeIdx == i)) {
            mGizmoModeIdx = i;
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

      if (auto nearPlane{mCam.GetNearClipPlane()}; ImGui::DragFloat("Near Clip Plane", &nearPlane,
        clipPlaneSliderSpeed, clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat,
        clipPlaneSliderFlags)) {
        mCam.SetNearClipPlane(nearPlane);
        mCam.SetFarClipPlane(std::max(nearPlane, mCam.GetFarClipPlane()));
      }

      if (auto farPlane{mCam.GetFarClipPlane()}; ImGui::DragFloat("Far Clip Plane", &farPlane,
        clipPlaneSliderSpeed, clipPlaneMin, clipPlaneMax, clipPlaneSliderFormat,
        clipPlaneSliderFlags)) {
        mCam.SetFarClipPlane(farPlane);
        mCam.SetNearClipPlane(std::min(farPlane, mCam.GetNearClipPlane()));
      }

      ImGui::DragFloat("Speed", &mCam.speed, 0.1f, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

      if (auto fov{mCam.GetVerticalPerspectiveFov()}; ImGui::SliderFloat("FOV", &fov, 5, 120, "%.0f", ImGuiSliderFlags_AlwaysClamp)) {
        mCam.SetVerticalPerspectiveFov(fov);
      }

      ImGui::EndPopup();
    }

    auto const contentRegionSize{ImGui::GetContentRegionAvail()};

    auto const rt{
      g_engine_context.render_manager->GetTemporaryRenderTarget(rendering::RenderTarget::Desc{
        .width = static_cast<UINT>(contentRegionSize.x),
        .height = static_cast<UINT>(contentRegionSize.y),
        .color_format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .depth_stencil_format = std::nullopt,
        .sample_count = 1,
        .debug_name = L"Scene View RT"
      })
    };

    mCam.SetRenderTarget(rt);

    auto const wasCamMoving{mCamMoving};
    mCamMoving = wasCamMoving ? ImGui::IsMouseDown(ImGuiMouseButton_Right) : ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (!wasCamMoving && mCamMoving) {
      g_engine_context.window->SetCursorLock(GetCursorPosition());
      g_engine_context.window->SetCursorHiding(true);
      ImGuizmo::Enable(false);
    } else if (wasCamMoving && !mCamMoving) {
      g_engine_context.window->SetCursorLock(std::nullopt);
      g_engine_context.window->SetCursorHiding(false);
      ImGuizmo::Enable(true);
    }

    if (mCamMoving) {
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

      mCam.position += mCam.orientation.Rotate(posDelta) * mCam.speed * timing::GetFrameTime();

      auto const [mouseX, mouseY]{GetMouseDelta()};
      auto constexpr sens{0.05f};

      mCam.orientation = Quaternion{Vector3::Up(), static_cast<f32>(mouseX) * sens} * mCam.orientation;
      mCam.orientation *= Quaternion{Vector3::Right(), static_cast<f32>(mouseY) * sens};
    }

    if (auto const selectedObject{context.GetSelectedObject()}) {
      selectedObject->OnDrawGizmosSelected();
    }

    ImGui::Image(rt->GetColorTex().get(), contentRegionSize);

    auto const aspect{ImGui::GetWindowWidth() / ImGui::GetWindowHeight()};
    auto const camViewMtx{mCam.CalculateViewMatrix()};
    auto const camProjMtx{mCam.CalculateProjectionMatrix(aspect)};

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();

    if (ImGui::IsWindowFocused() && GetKeyDown(Key::G)) {
      mShowGrid = !mShowGrid;
    }

    if (mShowGrid) {
      ImGuizmo::DrawGrid(camViewMtx.GetData(), camProjMtx.GetData(), Matrix4::Identity().GetData(),
        mCam.GetFarClipPlane());
    }

    if (mFocusTarget) {
      mFocusTarget->t += timing::GetFrameTime() * 5.0f;
      mFocusTarget->t = std::min(mFocusTarget->t, 1.0f);
      mCam.position = Lerp(mFocusTarget->source, mFocusTarget->target, mFocusTarget->t);

      if (mFocusTarget->t >= 1.0f) {
        mFocusTarget.reset();
      }
    }

    if (!context.GetImGuiIo().WantTextInput && !mCamMoving) {
      if (GetKeyDown(Key::Q)) {
        mGizmoOpIdx = 0;
      }
      if (GetKeyDown(Key::W)) {
        mGizmoOpIdx = 1;
      }
      if (GetKeyDown(Key::E)) {
        mGizmoOpIdx = 2;
      }
      if (GetKeyDown(Key::R)) {
        mGizmoModeIdx = 1 - mGizmoModeIdx;
      }
    }

    if (auto const selectedEntity{dynamic_cast<Entity*>(context.GetSelectedObject())}; selectedEntity) {
      if (!context.GetImGuiIo().WantTextInput && !mCamMoving && GetKeyDown(Key::F)) {
        mFocusTarget.emplace(mCam.GetPosition(), selectedEntity->GetTransform().GetWorldPosition() - mCam.GetForwardAxis() * 2, 0.0f);
      }

      // TODO this breaks if the transform is a child of a rotated transform
      if (Matrix4 modelMat{selectedEntity->GetTransform().GetLocalToWorldMatrix()}; Manipulate(camViewMtx.GetData(), camProjMtx.GetData(), GIZMO_OP_OPTIONS[mGizmoOpIdx].op, GIZMO_MODE_OPTIONS[mGizmoModeIdx].mode, modelMat.GetData())) {
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
  return mCam;
}
}
